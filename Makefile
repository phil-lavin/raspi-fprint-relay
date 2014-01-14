CC=gcc
CFLAGS=-I. -g
DEPS = raspi-fprint-relay.h lcd.h
OBJ = raspi-fprint-relay.o functions.o lcd.o
LIBS=-lwiringPi -lfprint
CURL_CONFIG=$(shell curl-config --libs)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raspi-fprint-relay: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(CURL_CONFIG)

lcd-test: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(CURL_CONFIG)

clean:
	rm -f raspi-fprint-relay *.o
