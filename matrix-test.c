#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <matrix.h>

void int_test() {
	printf("Interrupt!\n");
}

int main(void) {

	matrix_init(4, 17, 27, 22, 10, 9, 11);

	while (1) {
		sleep(1);
	}

	return 0;

	if (wiringPiSetupGpio() == -1) {
		printf("GPIO Setup Failed");
		return 1;
	}

	// Rows
	pinMode(22, INPUT);
	pullUpDnControl(22, PUD_DOWN);
	pinMode(10, INPUT);
	pullUpDnControl(10, PUD_DOWN);
	pinMode(9, INPUT);
	pullUpDnControl(9, PUD_DOWN);
	pinMode(11, INPUT);
	pullUpDnControl(11, PUD_DOWN);

	// Cols
	pinMode(4, OUTPUT);
	digitalWrite(4, HIGH);
	pinMode(17, OUTPUT);
	digitalWrite(17, HIGH);
	pinMode(27, OUTPUT);
	digitalWrite(27, HIGH);

	while (1) {
		wiringPiISR(9, INT_EDGE_RISING, int_test);
	}

	return 0;
}
