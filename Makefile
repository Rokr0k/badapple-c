CC = gcc

all: badapple

run: badapple video.mp4
	./$^

clean:
	$(RM) *.o
	$(RM) badapple

badapple: main.c
	$(CC) $^ -o $@
