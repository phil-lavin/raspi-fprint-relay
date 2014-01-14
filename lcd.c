#include <stdio.h>
#include <string.h>
#include <lcd.h>
#include <wiringPiI2C.h>

// Globals
int fd;
int lcd_io_register;
int lcd_pin_register;
int current_register_value;
int current_display_mode;

// Constants
#define RS_PIN 0x10
#define RW_PIN 0x20
#define E_PIN 0x40
#define DISPLAY_ON 0x04
#define CURSOR_ON 0x02
#define CURSOR_BLINK 0x01

/*
* Requires that the LCD screen be connected to GPIO pins controlled by the supplied register in the following order:
*
* 0 - DB4
* 1 - DB5
* 2 - DB6
* 3 - DB7
* 4 - RS
* 5 - R/W
* 6 - E
* 7 - Nothing Connected (you can't use this pin)
*/
void lcdinit(int io_register, int pin_register, int device) {
	lcd_io_register = io_register;
	lcd_pin_register = pin_register;

	fd = wiringPiI2CSetup(device);

	// All pins output and off
	reset_all();

	// Initial display mode
	current_display_mode = 0x0C; // Display on, cursor off, blink off

	write_4_bits(0x33, 0); // Init
	write_4_bits(0x32, 0); // Init / Change to 4 bit mode
	write_4_bits(0x28, 0); // 2 lines, 5x7
	write_4_bits(current_display_mode, 0); // Set initial display mode
	write_4_bits(0x06, 0); // Shift cursor Right

	clear_screen();
}

/*
* Writes a full byte to the pin register.
*
* This function should always be used in favour of wiringPiI2CWriteReg8() as it tracks current state
*/
void write_pins(int value) {
	wiringPiI2CWriteReg8(fd, lcd_pin_register, value);
	current_register_value = value;
}

/*
* Binary OR function. Pass a bitmask and it will OR it with the current pin register value and set that
* Should be used to turn on certain pins
*/
void write_pins_append(int value) {
	int tmp = current_register_value | value;
	write_pins(tmp);
}

/*
* Inverse of write_pins_append()
* Should be used to turn off certain pins
*/
void write_pins_remove(int value) {
	int tmp = current_register_value & ~value;
	write_pins(tmp);
}

/*
* Changes the state of the enabled pin
*/
void set_enable(int on) {
	if (on) {
		write_pins_append(E_PIN);
	}
	else {
		write_pins_remove(E_PIN);
	}
}

/*
* Changes the state of the RS pin
*/
void set_rs(int on) {
	if (on) {
		write_pins_append(RS_PIN);
	}
	else {
		write_pins_remove(RS_PIN);
	}
}

/*
* Sets all pins to output and turns them off
*/
void reset_all() {
	wiringPiI2CWriteReg8(fd, lcd_io_register, 0x00); // All pins output
	write_pins(0x00); // All pins off
}

/*
* Sends a pulse on the enable pin. This confirms the command sent to the LCD
*/
void pulse_enable() {
	set_enable(0);
	usleep(1);
	set_enable(1);
	usleep(1);
	set_enable(0);
	usleep(1);
}

/*
* Writes a byte to the pin register in 2 4 bit nibbles
* Uses append/remove functions such that it maintains current pin state
*/
void write_4_bits(int bits, int char_mode) {
	usleep(1000);

	reset_all();
	set_rs(char_mode);

	write_pins_append(bits>>4);

	pulse_enable();

	write_pins_remove(0xf);
	write_pins_append(bits&0xf);

	pulse_enable();

	usleep(500);
	reset_all();
}

/*
* Writes a single ASCII character to the LCD
*/
void write_char(char chr) {
	write_4_bits(chr, 1);
}

/*
* Writes a full ASCII string to the LCD
* Pass \n in the string to break onto the second line
*/
void write_string(char *str) {
	int i;
	int len;

	len = strlen(str);

	for (i = 0; i < len; i++) {
		if (*str == '\n') {
			write_4_bits(0xC0, 0);
		}
		else {
			write_char(*str);
		}

		str++;
	}
}

/*
* Clears the LCD
*/
void clear_screen() {
	write_4_bits(0x01, 0);
	usleep(3000); // This is apparently slow
}

/*
* Writes a display mode change. 3 things are controlled from the same command so the state must be remembered
* Change is the change bitmask and on is a 0/1 flag to denote if we're setting or unsetting this
*/
void write_display_mode_change(int change, int on) {
	// Add or remove bitmask from global
	if (on) {
		current_display_mode |= change;
	}
	else {
		current_display_mode &= ~change;
	}

	// Set
	write_4_bits(current_display_mode, 0);
}

/*
* Turns the cursor off
*/
void cursor_off() {
	write_display_mode_change(CURSOR_ON, 0);
}

/*
* Turns the cursor on
*/
void cursor_on() {
	write_display_mode_change(CURSOR_ON, 1);
}

/*
* Turns the screen off
*/
void screen_off() {
	write_display_mode_change(DISPLAY_ON, 0);
}

/*
* Turns screen on
*/
void screen_on() {
	write_display_mode_change(DISPLAY_ON, 1);
}

/*
* Turns cursor blink off
*/
void cursor_blink_off() {
	write_display_mode_change(CURSOR_BLINK, 0);
}

/*
* Turns cursor blink on
*/
void cursor_blink_on() {
	write_display_mode_change(CURSOR_BLINK, 1);
}

/*
* Moves cursor num spaces right
*/
void cursor_right(int num) {
	int i;

	for (i = 0; i < num; i++) {
		write_4_bits(0x14, 0);
	}
}

/*
* Moves cursor num spaces left
*/

void cursor_left(int num) {
	int i;

	for (i = 0; i < num; i++) {
		write_4_bits(0x10, 0);
	}
}
