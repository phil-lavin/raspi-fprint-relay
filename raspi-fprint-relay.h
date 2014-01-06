static void signal_handler(int sig);
void init_signals(void);
void close_open_devices();
int get_fingerprint_device(struct fp_dev **device);
void error(char *msg);
