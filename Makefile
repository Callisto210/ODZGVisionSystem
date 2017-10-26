CXX = g++
CFLAGS = -std=c++1z -pthread
OUTDIR = build
SRCDIR = src
LIBDIR = lib
INCLUDE = -I./include/ -I./src
LFLAGS = -lpistache -lpthread
CODECS = build/codecs/filter_graph.o build/codecs/codecs.o
REST = build/rest/endpoints.o
MAIN = build/main.o
COBJ = build/codecs/main.o build/codecs/filter_graph.c.o build/codecs/log_ffmpeg.o
OBJFILES = $(REST) $(MAIN) $(CODECS) $(COBJ)
CWD = $(shell pwd)


default: all

prepare:
	$(info Preparing directory structure)
	$(shell test -d build/rest || mkdir -p build/rest)
	$(shell test -d build/lib/pistache || mkdir -p build/lib/pistache)
	$(shell test -d bin/gst || mkdir -p bin/gst)
	$(shell test -d build/gst || mkdir -p build/gst)

rest: prepare bin/libpistache.a $(REST)

codecs: prepare $(CODECS)

link: bin/libpistache.a $(OBJFILES)
	$(CXX) $(CFLAGS) $(OBJFILES) -L bin -L bin/lib/ffmpeg -o bin/rest $(LFLAGS)
	$(info Linking...)
	$(info )


pistache: prepare lib/pistache
	$(info Building libpistache)
	@cd build/lib/pistache \
	&& cmake ../../../lib/pistache \
	&& make pistache && cd ../../../
	$(info Copying library file libpistache)
	@cp build/lib/pistache/src/libpistache.a bin/libpistache.a


libs: pistache

build/rest/%.o: src/rest/%.cc
	$(info Building rest objects)
	$(info )
	$(CXX) -c $(CFLAGS) $(INCLUDE) -o $@ $<


gst: lib/gstreamer
	$(info Building gstreamer library...)
	$(info Base directory... ${CWD})
	@cd ${CWD}/build/gst && ${CWD}/lib/gstreamer/autogen.sh --noconfigure \
	&& ${CWD}/lib/gstreamer/configure \
	--srcdir=${CWD}/lib/gstreamer \
	--libdir=${CWD}/bin/gst \
	--bindir=${CWD}/bin/gst/bin \
	--includedir=${CWD}/include \
	--disable-examples --disable-tests --disable-parse --disable-tools \
	--prefix=${CWD}/bin
	@cd  ${CWD}/build/gst && make all
	${MAKE} install


main:
	$(info Building main...)
	$(CXX) -c $(CFLAGS) $(INCLUDE) \
	-o $(OUTDIR)/main.o $(SRCDIR)/main.cc

clean:
	$(info Cleaning output files... )
	$(shell rm -rf $(OUTDIR)/* bin/*)


all: prepare libs rest codecs main link

.PHONY: clean objects default all pistache rest main
