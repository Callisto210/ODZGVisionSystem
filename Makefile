CXX = g++-6
CFLAGS = -std=c++1z -pthread
OUTDIR = build
SRCDIR = src
LIBDIR = lib
INCLUDE = -I./include/ -I./src
LFLAGS = -lpistache -lavformat -lavfilter -lavutil -lavresample -lswresample -lavdevice -lavcodec -lpthread -lswscale -lpostproc
CODECS = build/codecs/filter_graph.o build/codecs/codecs.o
FFOBJ = build/ffmpeg/AVFilter.o build/ffmpeg/AVCodec.o build/ffmpeg/AVPacket.o
REST = build/rest/endpoints.o
MAIN = build/main.o
COBJ = build/codecs/main.o build/codecs/filter_graph.c.o build/codecs/log_ffmpeg.o
OBJFILES = $(REST) $(MAIN) $(CODECS) $(FFOBJ) $(COBJ)
FFMPEG_BIN = bin/lib/ffmpeg
AVUTIL = $(FFMPEG_BIN)/libavformat.a $(FFMPEG_BIN)/libavformat.so $(FFMPEG_BIN)/libavformat.so.57


default: all

prepare:
	$(info Preparing directory structure)
	$(shell test -d build/rest || mkdir -p build/rest)
	$(shell test -d build/ffmpeg || mkdir -p build/ffmpeg)
	$(shell test -d build/lib/pistache || mkdir -p build/lib/pistache)
	$(shell test -d bin/lib/ffmpeg || mkdir -p bin/lib/ffmpeg)
	$(shell test -d build/codecs || mkdir -p build/codecs)

rest: prepare bin/libpistache.a $(REST)

codecs: prepare $(CODECS)

link: bin/libpistache.a $(OBJFILES)
	$(info Linking...)
	$(info )
	$(CXX) $(CFLAGS) $(OBJFILES) -L bin -L bin/lib/* -o bin/rest $(LFLAGS)


pistache: prepare lib/pistache
	$(info Building libpistache)
	@cd build/lib/pistache \
	&& cmake ../../../lib/pistache \
	&& make pistache && cd ../../../
	$(info Copying library file libpistache)
	@cp build/lib/pistache/src/libpistache.a bin/libpistache.a

ffmpeg: prepare lib/ffmpeg
	$(info Building ffmpeg libraries)
	@cd lib/ffmpeg && ./configure \
	--disable-programs --enable-gpl --libdir=../../bin/lib/ffmpeg \
	--shlibdir=../../bin/lib/ffmpeg \
	--enable-shared --enable-avresample --enable-libx264 --enable-libx265
	@cd ../../
	$(MAKE) -C lib/ffmpeg all
	$(info Copying libraries files)
	# Copying .a library files
	$(shell cp lib/ffmpeg/lib*/*.a bin/lib/ffmpeg)
	$(shell cp lib/ffmpeg/lib*/*.so* bin/lib/ffmpeg)



libs: pistache ffmpeg

build/rest/%.o: src/rest/%.cc
	$(info Building rest objects)
	$(info )
	$(CXX) -c $(CFLAGS) $(INCLUDE) -o $@ $<

build/codecs/%.o: src/codecs/%.cc
	$(info Building codecs objects)
	$(info )
	$(CXX) -c $(CFLAGS) $(INCLUDE) -o $@ $<

build/ffmpeg/%.o: src/ffmpeg/%.cc
	$(info Building ffmpeg Cpp wrapper classes)
	$(info )
	$(CXX) -c $(CFLAGS) $(INCLUDE) -o $@ $<


main:
	$(info Building main...)
	$(CXX) -c $(CFLAGS) $(INCLUDE) \
	-o $(OUTDIR)/main.o $(SRCDIR)/main.cc
	gcc -c -Wall -std=c99 -o build/codecs/main.o src/codecs/main.c $(INCLUDE)
	gcc -c -Wall -std=c99 -o build/codecs/filter_graph.c.o src/codecs/filter_graph.c $(INCLUDE)
	gcc -c -Wall -std=c99 -o build/codecs/log_ffmpeg.o src/codecs/log_ffmpeg.c $(INCLUDE)

clean:
	$(info Cleaning output files... )
	$(shell rm -rf $(OUTDIR)/* bin/*)


all: prepare libs rest codecs main link

.PHONY: clean objects default all pistache ffmpeg rest main link
