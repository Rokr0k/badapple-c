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
	printf("\033[?1049l\033[?25h");
	fflush(v_pipe);
	fclose(v_pipe);
}

void resize() {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	col = ws.ws_col;
	row = ws.ws_row;
}

void exit_handler(int sig) {
	(void)sig;
	exit(0);
}

void SIGWINCH_handler(int sig) {
	(void)sig;
	resize();
}

#define WIDTH 512
#define HEIGHT 384

int main(int argc, char** argv) {
	atexit(finish);
	signal(SIGTERM, exit_handler);
	signal(SIGINT, exit_handler);
	signal(SIGQUIT, exit_handler);
	signal(SIGWINCH, SIGWINCH_handler);
	FILE* v_pipe = popen("ffmpeg -hide_banner -loglevel error -i video.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "r");
	printf("\033[?1049h\033[?25l");
	resize();
	int i=0;
	int count;
	uint8_t frame[HEIGHT][WIDTH][3];
	int buffer[1000][500] = {0};
	clock_t a = clock();
	while(1) {
		count = fread(frame, 1, HEIGHT*WIDTH*3, v_pipe);
		if(count != HEIGHT*WIDTH*3) {
			break;
		}
		double hratio = WIDTH * 1.0 / col;
		double vratio = HEIGHT * 1.0 / row;
		for(int j=0; j<row; j++) {
			for(int k=0; k<col; k++) {
				buffer[j][k] = 0;
				int cnt=0;
				for(int jj=j*vratio; jj<(int)((j+1)*vratio); jj++) {
					for(int kk=k*hratio; kk<(int)((k+1)*hratio); kk++) {
						buffer[j][k] += frame[jj][kk][0]+frame[jj][kk][1]+frame[jj][kk][2];
						cnt++;
					}
				}
				buffer[j][k] /= cnt;
			}
		}
		usleep(1000000/30 - clock() + a);
		a = clock();
		for(int j=0; j<row; j++) {
			for(int k=0; k<col; k++) {
				printf("%c", colors[buffer[j][k]*strlen(colors)/766]);
			}
		}
		printf("\033[H");
		fflush(stdout);
	}
	exit(0);
	return 0;
}
