void matrix_init(int row1, int row2, int row3, int row4, int col1, int col2, int col3, void (*function)(char));
void reset();
void handle_row_interrupt(int pin);
int which_row(int pin);
int which_col(int pin);
void update_tri_code(char key);
char *get_tri_code();
void update_blocking_buffer(char key);
char *wait_for_keypress(char stop_char);
