#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <matrix.h>

int col_pins[3];
int row_pins[4];

void matrix_init(int col1, int col2, int col3, int row1, int row2, int row3, int row4) {
	col_pins[0] = col1;
	col_pins[1] = col2;
	col_pins[2] = col3;

	row_pins[0] = row1;
	row_pins[1] = row2;
	row_pins[2] = row3;
	row_pins[3] = row4;

	wiringPiSetupGpio();
	row_in();
}

void set_all_low() {
	int i;

	for (i = 0; i < 3; i++) {
		digitalWrite(col_pins[i], LOW);
	}

	for (i = 0; i < 4; i++) {
		digitalWrite(row_pins[i], LOW);
	}
}

void row_in() {
	int i;

	set_all_low();

	// Rows
	for (i = 0; i < 4; i++) {
		pinMode(row_pins[i], INPUT);
		pullUpDnControl(row_pins[i], PUD_DOWN);
	}

	// Cols
	for (i = 0; i < 3; i++) {
		pinMode(col_pins[i], OUTPUT);
		digitalWrite(col_pins[i], HIGH);
	}

	// Row interrupts
	for (i = 0; i < 4; i++) {
		wiringPiISR(row_pins[i], INT_EDGE_BOTH, handle_row_interrupt);
	}
}

void handle_row_interrupt(int pin) {
	printf("Interrupt on pin %d\n", pin);
}
