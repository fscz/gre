CC = gcc
AR = ar
CFLAGS = -Wall -g -O3 -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux/ -I../include
LDFLAGS = -shared -L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -pthread -lm
objects := $(patsubst %.c,%.o,$(wildcard *.c))

all: libgre.so libgre.a

.PHONY: clean 

clean:
	rm -Rf *.o
	rm -Rf *~
	rm -f libgre.a
	rm -f libgre.o

%.o: %.c
	${CC} ${CFLAGS} -c $<

libgre.so: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS) -Wl,--whole-archive ../lib/libgdsl.a -Wl,--no-whole-archive

libgre.a: $(objects)
	cd ../lib; ar -x libgdsl.a
	$(AR) rcs $@ $^ ../lib/*.o
	rm ../lib/*.o

