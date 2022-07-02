#pragma once

#include "reg_defs.h"

struct lcd_pixel {
    int com;
    int seg;
};

#define SET_NTH_BIT(reg, nth) ((reg) |= (1 << (nth)))

void display_pixel(struct lcd_pixel pix);
void fill_ram_buf();
void write_next_pixel();
void write_full_buf();
void commit_lcd_ram_buf();
void show_empty_screen();
void init_lcd();
int my_strlen(const char *str);
struct lcd_pixel map_pixel_alphabet(int digit, int alphabet);
const char *int_to_str(int num);
void write_string_to_ram_buf(const char *str);
void write_int_to_ram_buf(int num);

class Lcd {
public:
    void zero_ram_buf(void);
};
