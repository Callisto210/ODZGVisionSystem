CC=gcc
LIBS=-lavcodec -lavformat -lswscale -lavutil -lavfilter
CFLAGS=-Wall -Wextra -pedantic -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o )


all: $(OBJS)
	@echo $(LD_LIBRARY_PATH)
	$(CC) -L../../bin/lib/ffmpeg -Wl,-rpath-link=../../bin/lib/ffmpeg $(OBJS) -o main $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) -DDEBUG -I../../include

clean:
	rm -f *.o
	rm -f main
