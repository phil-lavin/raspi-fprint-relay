#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <matrix.h>

#define IGNORE_PRESS_INTERVAL_BELOW 200000

int col_pins[3];
int row_pins[4];
int interrupt_lock = 1;
int interrupts_registered = 0;
struct timeval last_press;
static void (*callback_function)(char);

char matrix_map[4][3] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};

void matrix_init(int row1, int row2, int row3, int row4, int col1, int col2, int col3, void (*function)(char)) {
	col_pins[0] = col1;
	col_pins[1] = col2;
	col_pins[2] = col3;

	row_pins[0] = row1;
	row_pins[1] = row2;
	row_pins[2] = row3;
	row_pins[3] = row4;

	callback_function = function;

	wiringPiSetupGpio();
	reset();
}

void reset() {
	int i;

	// Set last press time to now
	gettimeofday(&last_press, NULL);

	// Cols out, low
	for (i = 0; i < 3; i++) {
		pinMode(col_pins[i], OUTPUT);
		digitalWrite(col_pins[i], LOW);
	}

	// Rows
	for (i = 0; i < 4; i++) {
		pinMode(row_pins[i], INPUT);
	}

	// Row interrupts
	if (!interrupts_registered) {
		for (i = 0; i < 4; i++) {
			wiringPiISR(row_pins[i], INT_EDGE_FALLING, handle_row_interrupt);
		}

		interrupts_registered = 1;
	}

	// Unlock interrupt
	interrupt_lock = 0;
}

int which_row(int pin) {
	int i;

	for (i = 0; i < 4; i++) {
		if (row_pins[i] == pin) {
			return i;
		}
	}

	return 0;
}

int which_col(int pin) {
	int i;

	for (i = 0; i < 3; i++) {
		if (col_pins[i] == pin) {
			return i;
		}
	}

	return 0;
}

void handle_row_interrupt(int pin) {
	int i;
	int high_pin;
	int high_cnt;
	int current_row;
	struct timeval now;
	unsigned long diff;

	if (interrupt_lock) return;

	// Lock so we don't trigger the interrupt again while we're working
	interrupt_lock = 1;

	// Current time
	gettimeofday(&now, NULL);

	// Don't continue if we had a keypress in the last 0.1 seconds
	diff = (now.tv_sec * 1000000 + now.tv_usec) - (last_press.tv_sec * 1000000 + last_press.tv_usec);

	if (diff < IGNORE_PRESS_INTERVAL_BELOW) {
		interrupt_lock = 0;
		return;
	}

	// Get the row number for the interrupt pin
	current_row = which_row(pin);

	// Cols as input
	for (i = 0; i < 3; i++) {
		pinMode(col_pins[i], INPUT);
	}

	// Current row as output, high
	pinMode(current_row, OUTPUT);
	digitalWrite(current_row, HIGH);

	// Scan for a high col
	while (1) {
		/*
		 * This logic is a bit silly but it seems that, for whatever reason
		 * more than 1 pin can be high for a short period of time. The code
		 * loops until only 1 pin is high. This has been proven to produce
		 * accurate results regarding which key was pressed.
		*/
		high_cnt = 0;
		high_pin = 0;

		// Count number of high pins. Also record which pin is high so we can use
		// it when there's only 1 high pin
		for (i = 0; i < 3; i++) {
			if (digitalRead(col_pins[i]) == HIGH) {
				high_pin = i;
				high_cnt++;
			}
		}

		// If there's only 1 high pin, use it
		if (high_cnt == 1) {
			callback_function(matrix_map[current_row][high_pin]);
			break;
		}
	}

	// Reset
	reset();
}
