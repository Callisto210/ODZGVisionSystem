CC=gcc
LIBS=-lavcodec -lavformat -lswscale -lavutil -lavfilter
CFLAGS=-Wall -Wextra -pedantic

all: main.o
	$(CC) $< -o main $(LIBS)

main.o: main.c
	$(CC) -c $< -o $@ $(CFLAGS) -DDEBUG

clean:
	rm main
	rm *.o
