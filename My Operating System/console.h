#pragma once	//If you've seen a file before, always include it without checking
#include "Multiboot.h"

void console_init(struct MultibootInfo * m);
void set_pixel(int x, int y, int r, int g, int b);
void console_draw_char(int x, int y, char ch);
void console_draw_string(int x, int y, const char* s);
void console_putc(char c);
