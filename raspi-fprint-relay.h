static void signal_handler(int sig);
void init_signals(void);
void close_open_devices();
int get_fingerprint_device(struct fp_dev **device);
void error(char *msg);
void handle_key_press(char key);
void lcd_reset();
void write_admin_menu(int shift);

typedef enum {
	ENROLLMENT,
	WAITING,
	FRIDGE_OPEN
} state;