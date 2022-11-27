#pragma once

#include "reg_defs.hpp"
#include "reg_access.hpp"

struct lcd_pixel {
    int com;
    int seg;
};

#define SET_NTH_BIT(reg, nth) ((reg) |= (1 << (nth)))

class Lcd {
public:
    Lcd() : lcd_reg(get_lcd()){};

    const char *int_to_str(int num);
    const char *hex_to_str(int num);
    void reset(void);
    void write_string_to_ram_buf(const char *str);
    void commit();
    void init_lcd();
    void write_int_to_ram_buf(int num);

    // TODO: Make private after testing ready
    void display_alphabet_in_location(int alphabet, int location);
    void set_ram_buf(int ram_buf_idx, REG val);
    void set_ram_buf_bit(int ram_buf_idx, int bit_idx);

private:
    void display_pixel(struct lcd_pixel pix);
    void fill_ram_buf();
    void write_next_pixel();
    void write_full_buf();
    int my_strlen(const volatile char *str);
    struct lcd_pixel map_pixel_alphabet(int digit, int alphabet);
    void display_digit_in_location(int digit, int location);

    char digit_str[6] = {0, 0, 0, 0, 0, 0};
    // CPU side copy of the LCD pixel buffer. Always copied fully to LCD memory
    REG ram_buf[RAM_BUFS];
    int ram_pixel_idx = 0;
    int ram_buf_idx = 0;

    struct lcd *lcd_reg;
};
