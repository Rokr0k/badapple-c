CC = gcc

all: badapple

run: badapple
	./$^

clean:
	$(RM) *.o
	$(RM) badapple

badapple: main.c
	$(CC) $^ -o $@
