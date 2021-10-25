CC = gcc

all: badapple

run: badapple video.mp4
	./$^

clean:
	$(RM) *.o
	$(RM) badapple

badapple: main.o
	$(CC) $^ -o $@ -lc

main.o: main.c
	$(CC) -c $^ -o $@