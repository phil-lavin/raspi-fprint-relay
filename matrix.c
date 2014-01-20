#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <matrix.h>

#define IGNORE_PRESS_INTERVAL_BELOW 200000
#define TRI_CODE_PER_KEY_TIME 2000000

int col_pins[3];
int row_pins[4];
char tri_code[3];
char tri_code_str[4];
char *tri_code_pos;
char blocking_buffer[64];
char *blocking_buffer_pos;
int interrupt_lock = 1;
int interrupts_registered = 0;
int blocked = 0;
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

	tri_code_pos = &tri_code[0];
	blocking_buffer_pos = &blocking_buffer[0];

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

void update_tri_code(char key) {
	struct timeval now;
	unsigned long diff;

	gettimeofday(&now, NULL);

	diff = (now.tv_sec * 1000000 + now.tv_usec) - (last_press.tv_sec * 1000000 + last_press.tv_usec);

	// If we haven't had a key press in TRI_CODE_PER_KEY_TIME
	if (diff > TRI_CODE_PER_KEY_TIME) {
		tri_code_pos = &tri_code[0];
		tri_code[2] = 0;
	}

	// If the tri-code buffer is full, shift left
	if (tri_code_pos == &tri_code[2] && tri_code[2]) {
		memcpy(tri_code, tri_code + 1, 2);
		tri_code[2] = 0;
	}

	// Add key to the buffer
	*tri_code_pos = key;

	// Incremement the buffer, if we're not already at the end
	if (tri_code_pos != &tri_code[2]) {
		tri_code_pos++;
	}
}

/*
* Returns a pointer to the tri code. This is a null byte terminated char array.
* this is to a statically allocated buffer and thus does NOT need free()ing
*/
char *get_tri_code() {
	// Return NULL if there isn't a complete tri code
	if (tri_code_pos != &tri_code[2] || !tri_code[2]) {
		return NULL;
	}

	memcpy(tri_code_str, tri_code, 3);
	tri_code_str[3] = '\0';

	return tri_code_str;
}

/*
 * Updates the buffer for the wait_for_keypress function. Shifts left after 64 chars
*/
void update_blocking_buffer(char key) {
	// If the blocking buffer is full, shift left
	if (blocking_buffer_pos == &blocking_buffer[63] && blocking_buffer[63]) {
		memcpy(blocking_buffer, blocking_buffer + 1, 63);
		blocking_buffer[63] = 0;
	}

	// Incremement the buffer, if we're not already at the end and we've started
	if (blocking_buffer_pos != &blocking_buffer[63] && blocking_buffer[0]) {
		blocking_buffer_pos++;
	}

	// Add key to the buffer
	*blocking_buffer_pos = key;
}

/*
 * Blocking function that waits for a key press of a certain character and then returns all preceeding key presses
 * Returns a pointer to a statically allocated buffer. Do NOT free() it.
*/
char *wait_for_keypress(char stop_char) {
	// Start blocking
	blocked = 1;

	while (1) {
		// If the last character written was the stop char
		if (blocking_buffer_pos && *blocking_buffer_pos == stop_char) {
			// Strip the stop char
			*blocking_buffer_pos = 0;
			// Stop blocking
			blocked = 0;
			// Return a prt to the buffer
			return blocking_buffer;
		}

		// 0.001 seconds
		usleep(10000);
	}
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
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);

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
			if (!blocked) {
				update_tri_code(matrix_map[current_row][high_pin]);
				callback_function(matrix_map[current_row][high_pin]);
			}
			else {
				update_blocking_buffer(matrix_map[current_row][high_pin]);
			}

			break;
		}
	}

	// Reset
	reset();
}
