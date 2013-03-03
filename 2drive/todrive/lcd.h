void lcd_setup(char mode_8bit, char rs, char rw, char e, char *b, char lcdp);
void lcd_wr(char rs, char data);
char lcd_rd(char rs);
void lcd_str_pos(char *str, char off, char clr);
void lcd_chr(char c);
void lcd_clr();
void lcd_backlight(int on);
void lcd_on(int on);
void lcd_cursor(char cursor, char cursor_blink);
void lcd_pos(char off);
void lcd_move(char ln_r, char dist);

#define lcd_str(s) lcd_str_pos(s, 0, 1)
