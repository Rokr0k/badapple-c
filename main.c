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

#define WIDTH 512
#define HEIGHT 384
#define RATE 30

int tty = -1;

int min(int a, int b) {
	if(a>b)
		return b;
	else
		return a;
}

void finish() {
	fprintf(stdout, "\033[H\033[2J\033[?1049l\033[?25h\033[0m");
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

struct {
	int r;
	int g;
	int b;
	int n;
} color_schemes[] = {
	{0, 0, 0, 40},
	{170, 0, 0, 41},
	{0, 170, 0, 42},
	{170, 85, 0, 43},
	{0, 0, 170, 44},
	{170, 0, 170, 45},
	{0, 170, 170, 46},
	{170, 170, 170, 47}
};

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "no file input\n");
		exit(1);
	}
	signal(SIGTERM, exit_handler);
	signal(SIGINT, exit_handler);
	signal(SIGQUIT, exit_handler);
	signal(SIGWINCH, SIGWINCH_handler);
	if(tty < 0) {
		char* term = getenv("TERM");
		if(term) {
			if(strstr(term, "xterm")) {
				tty = 1;
			} else if(strstr(term, "linux")) {
				tty = 2;
			} else if(strstr(term, "vtnt")) {
				tty = 2;
			} else if(strstr(term, "cygwin")) {
				tty = 2;
			} else if(strstr(term, "vt220")) {
				tty = 0;
			} else if(strstr(term, "fallback")) {
				tty = 2;
			} else if(strstr(term, "rxvt-256color")) {
				tty = 1;
			} else if(strstr(term, "rxvt")) {
				tty = 2;
			} else if(strstr(term, "vt100") && col == 40) {
				tty = 0;
			} else if(!strncmp(term, "st", 2)) {
				tty = 1;
			} else {
				tty = 2;
			}
		}
	}
	char* command = calloc(112 + strlen(argv[1]), sizeof(char));
	sprintf(command, "ffmpeg -hide_banner -loglevel warning -i \"%s\" -s %dx%d -r %d -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", argv[1], WIDTH, HEIGHT, RATE);
	v_pipe = popen(command, "r");
	free(command);
	resize();
	fprintf(stdout, "\033[?1049h\033[?25l");
	int count;
	uint8_t frame[HEIGHT][WIDTH][3];
	int buffer[200][500][3];
	clock_t a = clock();
	while(1) {
		count = fread(frame, 1, HEIGHT * WIDTH * 3, v_pipe);
		if(count != HEIGHT*WIDTH*3) {
			break;
		}
		double hratio = WIDTH * 1.0 / col;
		double vratio = HEIGHT * 1.0 / row;
		for(int i = 0; i < row; i++) {
			for(int j = 0; j < col; j++) {
				switch(tty) {
				case 1:
					{
					buffer[i][j][0] = 0;
					buffer[i][j][1] = 0;
					buffer[i][j][2] = 0;
					int cnt=0;
					for(int ii = i * vratio; ii < (int)((i + 1) * vratio); ii++) {
						for(int jj = j * hratio; jj < (int)((j + 1) * hratio); jj++) {
							buffer[i][j][0] += frame[ii][jj][0];
							buffer[i][j][1] += frame[ii][jj][1];
							buffer[i][j][2] += frame[ii][jj][2];
							cnt++;
						}
					}
					buffer[i][j][0] /= cnt;
					buffer[i][j][1] /= cnt;
					buffer[i][j][2] /= cnt;
					}
					break;
				case 2:
					{
					int r=0, g=0, b=0;
					int cnt=0;
					for(int ii = i * vratio; ii < (int)((i + 1) * vratio); ii++) {
						for(int jj = j * hratio; jj < (int)((j + 1) * hratio); jj++) {
							r += frame[ii][jj][0];
							g += frame[ii][jj][1];
							b += frame[ii][jj][2];
							cnt++;
						}
					}
					r /= cnt;
					g /= cnt;
					b /= cnt;
					unsigned int d = 4294967295;
					buffer[i][j][0] = 0;
					for(int k=0; k<8; k++) {
						int dr = color_schemes[k].r - r;
						int dg = color_schemes[k].g - g;
						int db = color_schemes[k].b - b;
						unsigned int _d = dr*dr+dg*dg+db*db;
						if(_d < d) {
							d = _d;
							buffer[i][j][0] = color_schemes[k].n;
						}
					}
					}
					break;
				default:
					{
					buffer[i][j][0] = 0;
					buffer[i][j][1] = 0;
					buffer[i][j][2] = 0;
					int cnt=0;
					for(int ii = i * vratio; ii < (int)((i + 1) * vratio); ii++) {
						for(int jj = j * hratio; jj < (int)((j + 1) * hratio); jj++) {
							buffer[i][j][0] += frame[ii][jj][0] + frame[ii][jj][1] + frame[ii][jj][2];
							cnt++;
						}
					}
					buffer[i][j][0] /= cnt;
					}
				}
			}
		}
		usleep(1000000 / RATE - (clock() - a) * 1000000 / CLOCKS_PER_SEC);
		a = clock();
		for(int i = 0; i < row; i++) {
			for(int j = 0; j < col; j++) {
				switch(tty) {
				case 1:
					fprintf(stdout, "\033[48;2;%d;%d;%dm ", buffer[i][j][0], buffer[i][j][1], buffer[i][j][2]);
					break;
				case 2:
					fprintf(stdout, "\033[%dm ", buffer[i][j][0]);
					break;
				default:
					fprintf(stdout, "%c", colors[buffer[i][j][0]*strlen(colors)/766]);
				}
			}
		}
		fprintf(stdout, "\033[H");
		fflush(stdout);
	}
	finish();
	return 0;
}
