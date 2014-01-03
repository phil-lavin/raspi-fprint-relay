CC=gcc
CFLAGS=-I.
DEPS = raspi-fprint-relay.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: raspi-fprint-relay.o functions.o
	gcc -o raspi-fprint-relay raspi-fprint-relay.o functions.o -I.

clean:
	rm -f *.o raspi-fprint-relay