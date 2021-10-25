#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>

const char colors[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

int row, col;

FILE* v_pipe;

int min(int a, int b) {
	if(a>b)
		return b;
	else
		return a;
}

void finish() {
	fprintf(stdout, "\033[?1049l\033[?25h");
	fflush(v_pipe);
	pclose(v_pipe);
	exit(0);
}

void resize() {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	col = ws.ws_col;
	row = ws.ws_row;
}

void exit_handler(int sig) {
	(void)sig;
	finish();
}

void SIGWINCH_handler(int sig) {
	(void)sig;
	resize();
}

#define WIDTH 512
#define HEIGHT 384
#define RATE 30

int main(int argc, char** argv) {
	if(argc <= 1) {
		fprintf(stderr, "no file input\n");
		exit(1);
	}
	signal(SIGTERM, exit_handler);
	signal(SIGINT, exit_handler);
	signal(SIGQUIT, exit_handler);
	signal(SIGWINCH, SIGWINCH_handler);
	char* command = calloc(112 + strlen(argv[1]), sizeof(char));
	sprintf(command, "ffmpeg -hide_banner -loglevel warning -i \"%s\" -s %dx%d -r %d -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", argv[1], WIDTH, HEIGHT, RATE);
	v_pipe = popen(command, "r");
	free(command);
	resize();
	fprintf(stdout, "\033[?1049h\033[?25l");
	int i=0;
	int count;
	uint8_t frame[HEIGHT][WIDTH][3];
	int buffer[200][500][3] = {0};
	clock_t a = clock();
	while(1) {
		count = fread(frame, 1, HEIGHT * WIDTH * 3, v_pipe);
		if(count != HEIGHT*WIDTH*3) {
			break;
		}
		double hratio = WIDTH * 1.0 / col;
		double vratio = HEIGHT * 1.0 / row;
		for(int j = 0; j < row; j++) {
			for(int k = 0; k < col; k++) {
				buffer[j][k][0] = 0;
				buffer[j][k][1] = 0;
				buffer[j][k][2] = 0;
				int cnt=0;
				for(int jj = j * vratio; jj < (int)((j + 1) * vratio); jj++) {
					for(int kk = k * hratio; kk < (int)((k + 1) * hratio); kk++) {
						buffer[j][k][0] += frame[jj][kk][0];
						buffer[j][k][1] += frame[jj][kk][1];
						buffer[j][k][2] += frame[jj][kk][2];
						cnt++;
					}
				}
				buffer[j][k][0] /= cnt;
				buffer[j][k][1] /= cnt;
				buffer[j][k][2] /= cnt;
			}
		}
		usleep(1000000 / RATE - (clock() - a) * 1000000 / CLOCKS_PER_SEC);
		a = clock();
		for(int j = 0; j < row; j++) {
			for(int k = 0; k < col; k++) {
				fprintf(stdout, "\033[48;2;%d;%d;%dm ", buffer[j][k][0], buffer[j][k][1], buffer[j][k][2]);
			}
		}
		fprintf(stdout, "\033[H");
		fflush(stdout);
	}
	finish();
	return 0;
}
