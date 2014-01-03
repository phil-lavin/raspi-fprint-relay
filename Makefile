CC=gcc
CFLAGS=-I.
DEPS = raspi-fprint-relay.h
OBJ = raspi-fprint-relay.o functions.o
LIBS=-lwiringPi -lfprint
CURL_CONFIG=$(shell curl-config --libs)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raspi-fprint-relay: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(CURL_CONFIG)

clean:
	rm -f raspi-fprint-relay *.o
