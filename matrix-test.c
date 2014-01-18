#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <matrix.h>

void handle_key_press(char key) {
	printf("Key press of %c\n", key);
}

int main(void) {
	matrix_init(4, 17, 27, 22, 10, 9, 11, handle_key_press);

	while (1) {
		sleep(1);
	}

	return 0;
}
