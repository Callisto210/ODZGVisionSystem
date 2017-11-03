CC=gcc
CXX=g++
LIBS=`pkg-config --libs gstreamer-1.0 gstreamer-audio-1.0 gstreamer-video-1.0`
GST_CFLAGS=`pkg-config --cflags gstreamer-1.0 gstreamer-audio-1.0 gstreamer-video-1.0`
CFLAGS=-Wall -Wextra -pedantic -g -std=c99
INCLUDE=-Ilib/
SRCS=$(wildcard src/*.c) $(wildcard src/codecs/*.c)
OBJS=$(SRCS:.c=.o )

all: $(OBJS) jsmn
	$(info Libs: $(LIBS))
	$(CC) $(OBJS)  -o main -L./lib/jsmn -ljsmn $(LIBS)

%.o: %.c
	$(info GST_FLAGS: $(GST_FLAGS))
	$(CC) -c $< -o $@ $(CFLAGS) $(GST_CFLAGS) $(INCLUDE) 


jsmn:
	$(info Building jsmn library)
	@cd build && $(MAKE) -C ../lib/jsmn all && cd ..
	

clean:
	$(MAKE) -C lib/jsmn clean
	rm -fv *.o
	rm -fv src/*.o
	rm -fv src/codecs/*.o
	rm main

.PHONY:
	clean all jsmn
