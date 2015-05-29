CC = gcc
AR = ar

all: setup build_lib build_src build_binding copy

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

build_lib:
	cd lib; make; 
	cd lib; ar cr libgdsl.a gdsl-1.8/src/*.o

build_src: build_lib
	cd src; make

build_binding: build_src
	cd binding; make

install: build/libgre.so
	cp build/libgdsl.so /usr/local/lib
	mkdir -p /usr/local/bin/include
	cp -a build/include/* /usr/local/include

copy:
	cp src/*.h  build/include
	cp src/libgre.so build
	cp binding/pygre.so build

test: build_src build_binding
	cd test; make clean; make; ./runTests