#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libfprint/fprint.h>
#include <curl/curl.h>
#include <raspi-fprint-relay.h>

int main(void) {
	// Vars
	struct fp_dev *device;
	struct fp_driver *driver;
	struct fp_print_data *print_data;
	struct fp_img *img;
	int err;

	// Init libfprint
	fp_init();

	// Signal handler - does an fp_exit()
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
