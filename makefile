CC=gcc
LIBS=`pkg-config --libs gstreamer-1.0 gstreamer-audio-1.0 gstreamer-video-1.0`
GST_CFLAGS=`pkg-config --cflags gstreamer-1.0 gstreamer-audio-1.0 gstreamer-video-1.0`
CFLAGS=-Wall -Wextra -pedantic -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o )

all: $(OBJS)
	$(CC) $(OBJS) jsmn/libjsmn.a -o main $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(GST_CFLAGS) 

clean:
	rm *.o
	rm main
