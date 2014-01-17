CC=gcc
CFLAGS=-I. -g
DEPS = raspi-fprint-relay.h lcd.h matrix.h
OBJ = raspi-fprint-relay.o functions.o lcd.o matrix.o
LCD_TEST_OBJ = lcd-test.o lcd.o
MATRIX_TEST_OBJ = matrix-test.o matrix.o
LIBS=-lwiringPi -lfprint
CURL_CONFIG=$(shell curl-config --libs)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raspi-fprint-relay: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(CURL_CONFIG)

lcd-test: $(LCD_TEST_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(CURL_CONFIG)

matrix-test: $(MATRIX_TEST_OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS) $(CURL_CONFIG)

clean:
	rm -f raspi-fprint-relay lcd-test *.o
