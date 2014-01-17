#include <stdlib.h>
#include <lcd.h>
#include <wiringPiI2C.h>

int main(void) {
	lcdinit(0x00, 0x12, 0x20);
	//display_one_line();
	clear_screen();
	write_string("How Many Cans? 1");
	cursor_left(1);
	cursor_blink_on();
	sleep(3);
//	display_two_lines();
	write_char('5');
	cursor_left(1);
	sleep(3);
	return 1;
}
