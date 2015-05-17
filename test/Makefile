CC = gcc
AR = ar
CFLAGS = -Wall -g -O3 -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux/ -I../src/
LDFLAGS = -L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -pthread -lm ../src/libgre.a
objects := $(patsubst %.c,%.o,$(wildcard *.c))

all: runTests

.PHONY: clean 

clean:
	rm -Rf *.o
	rm -Rf runTests

%.o: %.c
	${CC} ${CFLAGS} -c $<

runTests: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS) 
