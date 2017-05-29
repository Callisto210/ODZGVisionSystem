CC=gcc
LIBS=-lavcodec -lavformat -lswscale -lavutil
CFLAGS=-Wall -Wextra -pedantic

all: main.o
	$(CC) $< -o main $(LIBS)

main.o: main.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm main
	rm *.o
