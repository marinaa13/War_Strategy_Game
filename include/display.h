#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void display_init(void);
void display_clear(void);
void display_set_cursor(uint8_t col, uint8_t page);
void display_print(const char *str);
void display_draw_bitmap_128x64(const uint8_t *bitmap);

#endif