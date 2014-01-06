#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libfprint/fprint.h>
#include <curl/curl.h>
#include <raspi-fprint-relay.h>

// Init signal handler
void init_signals(void) {
	struct sigaction sigact;

	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);
}

// Signal handler
static void signal_handler(int sig) {
	// Ctrl + C
	if (sig == SIGINT) {
		fp_exit();
		exit(1);
	}
}

// Get the first fingerprint device
// @return int	< 0 denotes an error	> 0 denotes success
int get_fingerprint_device(struct fp_dev **device) {
    struct fp_dscv_dev **devices;
    int rtn;

	// Discover devices
    if (!(devices = fp_discover_devs())) {
        rtn = -1;
    }
    else {
    	// Open the first discovered device
        if (!(*device = fp_dev_open(*devices))) {
            rtn = -2;
        }
        else {
			rtn = 1;
        }

		// Free discovered devices
        fp_dscv_devs_free(devices);
    }

    return rtn;
}

// Prints an error
void error(char *msg) {
	// Print to stderr
	fprintf(stderr, "%s\n", msg);
}
