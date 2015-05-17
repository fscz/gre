CC = gcc
AR = ar
CFLAGS = -Wall -g -O3 -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux/ -Iinclude
LDFLAGS = -shared -L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -pthread -lm
objects := $(patsubst %.c,%.o,$(wildcard src/*.c))

all: setup build_src build_binding copy

.PHONY: clean setup copy

clean:
	rm -Rf build
	rm -Rf src/*.o
	rm -Rf src/*~
	rm -Rf src/*.a
	rm -Rf src/*.so
	rm -Rf test/*.o
	rm -Rf test/*~
	rm -Rf binding/*.o	
	rm -Rf binding/*~

setup:
	mkdir -p build/include


build_src:
	cd src; make

build_binding: build_src
	cd binding; make

install: build/libgre.so
	cp build/libgdsl.so /usr/local/lib
	mkdir -p /usr/local/bin/include
	cp -a build/include/* /usr/local/include

copy:
	cp src/*.h  build/include
	cp src/libgre.a build
	cp src/libgre.so build
	cp binding/pygre.so build

test: build_src build_binding
	cd test; make clean; make; ./runTests