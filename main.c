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

int min(int a, int b) {
	if(a>b)
		return b;
	else
		return a;
}

void finish() {
	printf("\033[2J\033[H");
	system("rm -rf temp");
	exit(0);
}

void resize() {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	col = ws.ws_col;
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

int main(int argc, char** argv) {
	char* video = "video.mp4";
	if(argc > 1) {
		video = argv[1];
	}
	signal(SIGINT, SIGINT_handler);
	signal(SIGWINCH, SIGWINCH_handler);
	system("mkdir temp");
	char* parser = malloc(sizeof(char) * (64 + strlen(video)));
	sprintf(parser, "ffmpeg -hide_banner -i \"%s\" -s 720x480 -vf fps=15 temp/%%04d.bmp", video);
	system(parser);
	free(parser);
	printf("\033[2J\033[H");
	resize();
	int i=0;
	FILE* bmp = NULL;
	char file[20]={0};
	uint8_t* pixels = NULL;
	int buffer[1000][500] = {0};
	int width, height, bytesPerPixel;
	clock_t a = clock();
	while(1) {
		sprintf(file, "temp/%04d.bmp", ++i);
		bmp = fopen(file, "rb");
		if(bmp == NULL) {
			break;
		}
		int dataOffset;
		fseek(bmp, 0x0A, SEEK_SET);
		fread(&dataOffset, sizeof(int), 1, bmp);
		fseek(bmp, 0x12, SEEK_SET);
		fread(&width, sizeof(int), 1, bmp);
		fseek(bmp, 0x16, SEEK_SET);
		fread(&height, sizeof(int), 1, bmp);
		int16_t bitsPerPixel;
		fseek(bmp, 0x1C, SEEK_SET);
		fread(&bitsPerPixel, sizeof(int16_t), 1, bmp);
		bytesPerPixel = ((int)bitsPerPixel) / 8;
        int paddedRowSize = width*bytesPerPixel;
        int unpaddedRowSize = width*bytesPerPixel;
		pixels = malloc(unpaddedRowSize * height);
		uint8_t* currentRowPtr = pixels + ((height-1)*unpaddedRowSize);
		for(int j=0; j<height; j++) {
			fseek(bmp, dataOffset+(j*paddedRowSize), SEEK_SET);
			fread(currentRowPtr, sizeof(uint8_t), unpaddedRowSize, bmp);
			currentRowPtr -= unpaddedRowSize;
		}
		fclose(bmp);
		double hratio = width * 1.0 / col;
		double vratio = height * 1.0 / row;
		for(int j=0; j<row; j++) {
			for(int k=0; k<col; k++) {
				buffer[j][k] = 0;
				int cnt=0;
				for(int jj=j*vratio; jj<(int)((j+1)*vratio); jj++) {
					for(int kk=k*hratio; kk<(int)((k+1)*hratio); kk++) {
						buffer[j][k] += pixels[3*(jj*width+kk)] + pixels[3*(jj*width+kk) + 1] + pixels[3*(jj*width+kk) + 2];
						cnt++;
					}
				}
				buffer[j][k] /= cnt;
			}
		}
		free(pixels);
		usleep(1000000/15 - clock() + a);
		a = clock();
		for(int j=0; j<row; j++) {
			for(int k=0; k<col; k++) {
				printf("%c", colors[buffer[j][k]*strlen(colors)/766]);
			}
			printf("\n");
		}
		printf("\033[H");
	}
	finish();
	return 0;
}
