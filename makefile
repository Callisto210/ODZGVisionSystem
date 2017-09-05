CC=gcc
LIBS=-lavcodec -lavformat -lswscale -lavutil -lavfilter
CFLAGS=-Wall -Wextra -pedantic -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o )

all: $(OBJS)
	$(CC) $(OBJS) -o main $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) -DDEBUG

clean:
	rm *.o
	rm main
