CC = gcc

all: badapple

clean:
	$(RM) *.o
	$(RM) badapple

badapple: main.c
	$(CC) $^ -o $@ -lavcodec
