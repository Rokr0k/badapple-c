#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>

const char colors[] = " .:-=+*%#@";

int row, col;

int min(int a, int b) {
	if(a>b)
		return b;
	else
		return a;
}

void finish() {
	printf("\033[2J\033[H");
	system("rm -rf frames");
	exit(0);
}

void resize() {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	col = ws.ws_col - 1;
	row = ws.ws_row - 1;
}

void SIGINT_handler(int sig) {
	(void)sig;
	finish();
}

void SIGWINCH_handler(int sig) {
	(void)sig;
	resize();
}

int main() {
	signal(SIGINT, SIGINT_handler);
	signal(SIGWINCH, SIGWINCH_handler);
	system("mkdir frames");
	system("ffmpeg -i badapple.avi -vf fps=15 frames/%04d.bmp");
	resize();
	int i=0;
	FILE* bmp = NULL;
	char file[20]={0};
	uint8_t info[54];
	uint8_t* r = NULL;
	uint8_t data[600][500] = {0};
	int buffer[500][400] = {0};
	int width = 0, height = 0;
	clock_t a = clock();
	while(1) {
		sprintf(file, "frames/%04d.bmp", ++i);
		bmp = fopen(file, "r");
		if(bmp == NULL) {
			break;
		}
		fread(info, sizeof(uint8_t), 54, bmp);
		width = *(int*)&info[18];
		height = *(int*)&info[22];
		int hratio = width / col;
		int vratio = height / row;
		int row_padding = (width*3+3) & (~3);
		r = malloc(sizeof(uint8_t)*row_padding);
		for(int j=0; j<height; j++) {
			fread(r, sizeof(uint8_t), row_padding, bmp);
			for(int k=0; k<width; k++) {
				data[j][k] = r[k*3];
			}
		}
		free(r);
		fclose(bmp);
		for(int j=0; j<row; j++) {
			for(int k=0; k<col; k++) {
				buffer[row-j-1][k] = 0;
				int cnt=0;
				for(int jj=j*vratio; jj<(j+1)*vratio; jj++) {
					for(int kk=k*hratio; kk<(k+1)*hratio; kk++) {
						buffer[row-j-1][k] += data[jj][kk];
						cnt++;
					}
				}
				buffer[row-j-1][k] /= cnt;
			}
		}
		usleep(1000000/15/2 - clock() + a);
		a = clock();
		for(int j=0; j<row; j++) {
			for(int k=0; k<col; k++) {
				printf("%c", colors[buffer[j][k]*strlen(colors)/256]);
			}
			printf("\n");
		}
		printf("\033[H");
	}
	finish();
	return 0;
}