CC = gcc
AR = ar
CFLAGS = -Wall -g -O3 -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux/ -I/usr/include/python2.7 -I../src/
LDFLAGS = -shared -L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -pthread -lm
objects := $(patsubst %.c,%.o,$(wildcard *.c))

all: pygre.so 

.PHONY: clean

clean:
	rm -Rf *.o
	rm -Rf *~


%.o: %.c
	${CC} ${CFLAGS} -c $<

pygre.so: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS) -Wl,--whole-archive ../src/libgre.a -Wl,--no-whole-archive
