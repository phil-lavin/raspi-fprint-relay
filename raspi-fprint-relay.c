#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libfprint/fprint.h>
#include <curl/curl.h>
#include <raspi-fprint-relay.h>
#include <wiringPiI2C.h>

// Global vars
state system_state = ENROLLMENT;
int lcd_state;
int admin_line;

#define LCD_STATE_DEFAULT 0
#define LCD_STATE_ADMIN 1

int main(void) {
	// Vars
	struct fp_dev *device;
	struct fp_driver *driver;
	struct fp_print_data *print_data;
	struct fp_img *img;
	int err;

	// Init the LCD
	lcdinit(0x00, 0x12, 0x20);
	lcd_reset();

	// Init libfprint
	fp_init();

	// Init the keypad
	matrix_init(4, 17, 27, 22, 10, 9, 11, handle_key_press);

	// Signal handler - does an fp_exit() on SIGINT
	init_signals();

	// Get the first fingerprint device
	if ((err = get_fingerprint_device(&device)) < 0 || !device) { // Errorz
		if (err == -1) {
			error("No devices found");
		}
		else if (err == -2) {
			error("Couldn't open the device");
		}

		return 1;
	}

	// Get driver
	driver = fp_dev_get_driver(device);

	// Init message
	printf("Programme started! Device is %s\n\n", fp_driver_get_full_name(driver));

	// Scan the print
	fp_enroll_finger_img(device, &print_data, &img);

	// Deinit libfprint
	fp_exit();

	return 0;
}

void lcd_reset() {
	clear_screen();
	write_string("How Many Cans? 1");
	cursor_left(1);
	cursor_on();
	cursor_blink_on();

	lcd_state = LCD_STATE_DEFAULT;
}

void write_admin_menu(int shift) {
	int i;

	// The options available
	const char *options[] = {
		"1. Restock",
		"2. Enroll",
		"3. Deal Drugs"
	};
	const int num_options = 3; // Should be the number of things in the above

	// Don't let us shift more than the number of options
	if (shift > num_options) {
		return;
	}

	// Whealer's fault. Shift options around in a circle
	admin_line += shift + num_options;
	admin_line %= num_options;

	// Clean up
	clear_screen();
	cursor_blink_off();
	cursor_off();

	// Write the first thing
	write_string(options[admin_line]);

	// Write the next thing - go around in a circle. Also Whealer's fault
	write_string("\n");
	write_string(options[(admin_line + 1) % num_options]);
}

void handle_key_press(char key) {
	char *tri_code;

	// Standard key presses...

	// Default "How Many Cans?" screen
	if (lcd_state == LCD_STATE_DEFAULT) {
		// 1 - 9
		if (key >= 49 && key <= 57) {
			write_char(key);
			cursor_left(1);
		}
	}

	// Admin menu
	if (lcd_state == LCD_STATE_ADMIN) {
		if (key == '#') {
			write_admin_menu(1);
		}
		else if (key == '*') {
			write_admin_menu(-1);
		}
		// 1 - 9
		if (key >= 49 && key <= 57) {
			switch (key) {
				// Restock
				case '1':
					// Unlock teh door
					break;
				// Enroll
				case '2':
					// Enroll
					break;
				// Drug deal
				case '3':
					// Deduct from user 'drugs'
					// Open door
					break;
			};
		}
	}

	// Tri codes...

	// *69 for admin
	if (tri_code = (char *)get_tri_code()) {
		if (strcmp(tri_code, "*69") == 0) {
			clear_screen();
			cursor_blink_off();
			cursor_off();
			write_string("Admin mode...\n* & # to scroll");

			admin_line = 0;

			sleep(2);

			write_admin_menu(0);

			lcd_state = LCD_STATE_ADMIN;
		}
	}
}
