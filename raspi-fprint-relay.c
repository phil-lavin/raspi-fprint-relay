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
	cursor_blink_on();

	lcd_state = 0;
}

void handle_key_press(char key) {
	char *tri_code;

	// Default "How Many Cans?" screen
	if (lcd_state == 0) {
		// 1 - 9
		if (key >= 49 && key <= 57) {
			write_char(key);
			cursor_left(1);
		}
	}

	if (tri_code = (char *)get_tri_code()) {
		if (strcmp(tri_code, "*69") == 0) {
			clear_screen();
			write_string("Admin MOFO!\nKeith Wheal");
		}
	}

	printf("Key press of %c\n", key);
}
