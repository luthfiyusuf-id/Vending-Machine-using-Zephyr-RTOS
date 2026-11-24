#ifndef LCD_H
#define LCD_H

#include <stdint.h>
void lcd_clear_line(uint8_t row);
int lcd_init(void);

int lcd_clear(void);

int lcd_set_cursor(uint8_t row, uint8_t col);

int lcd_print(const char *text);

int lcd_print_at(uint8_t row, uint8_t col,
                 const char *text);

#endif