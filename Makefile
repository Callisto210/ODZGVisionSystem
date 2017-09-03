CXX=g++-7
CFLAGS=-std=c++1z -pthread
OUTDIR=build
SRCDIR=src
LIBDIR=lib
INCLUDE=-I./include/
LFLAGS=-lpistache -lavfromat -lavfilter -libav

REST= build/rest/endpoints.o
MAIN=build/main.o
OBJFILES = $(REST) $(MAIN)
default: all

prepare:
	$(info Preparing directory structure)
	$(shell test -d build/rest || mkdir build/rest)
	$(shell test -d build/lib/pistache || mkdir -p build/lib/pistache)
	$(shell test -d bin/lib/ffmpeg || mkdir -p bin/lib/ffmpeg)

rest: prepare bin/libpistache.a $(REST)


link: bin/libpistache.a $(OBJFILES)
	$(info Linking...)
	$(info )
	$(CXX) $(CFLAGS) $(OBJFILES) -L bin -l pistache -o bin/rest


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


libs: pistache ffmpeg

build/rest/%.o: src/rest/%.cc
	$(info Building rest objects)
	$(info)
	$(CXX) -c $(CFLAGS) $(INCLUDE) -o $@ $<


main:
	$(info Building main...)
#	$(CXX) -c $(CFLAGS) $(INCLUDE) -I/Linux/Projekty/pistache/include \
#	-o $(OUTDIR)/endpoints.o $(SRCDIR)/rest/endpoints.cc
#	$(info  )
	$(CXX) -c $(CFLAGS) $(INCLUDE) \
	-o $(OUTDIR)/main.o $(SRCDIR)/main.cc

clean:
	$(info Cleaning output files... )
	$(shell rm -rf $(OUTDIR)/* bin/*)


all: prepare libs rest main link

.PHONY: clean objects default all pistache ffmpeg rest main link