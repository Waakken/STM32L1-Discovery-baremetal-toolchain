#pragma once

#include "reg_accessor.hpp"
#include "reg_defs.hpp"

struct lcd_pixel {
    int com;
    int seg;
};

#define SET_NTH_BIT(reg, nth) ((reg) |= (1 << (nth)))

class Lcd : public RegAccessor<Lcd>
{
public:
    Lcd()
        : lcd_reg(get_lcd()){};

    void int_to_str(int num);
    void hex_to_str(int num);
    void str_to_str(const char *str);
    void reset_ram_buf(void);
    // TODO: Does this need to be public? It seems to complicate the interface
    void write_string_to_ram_buf(void);
    void commit() const;
    void init_lcd() const;
    void write_int_to_ram_buf(int num);

    // TODO: Make private after testing ready
    void display_alphabet_in_location(int alphabet, int location);
    void set_ram_buf(int ram_buf_idx, REG val);
    void set_ram_buf_bit(int ram_buf_idx, int bit_idx);
    static int my_strlen(const volatile char *str);

private:
    void clear_digit_str();
    void display_pixel(struct lcd_pixel pix);
    void fill_ram_buf();
    void write_next_pixel();
    void write_full_buf();
    struct lcd_pixel map_pixel_alphabet(int digit, int alphabet) const;
    void display_digit_in_location(int digit, int location);

    static constexpr unsigned digit_str_len{6};
    char digit_str[digit_str_len] = {0, 0, 0, 0, 0, 0};
    // CPU side copy of the LCD pixel buffer. Always copied fully to LCD memory
    REG ram_buf[RAM_BUFS];
    int ram_pixel_idx = 0;
    int ram_buf_idx = 0;

    struct lcd *lcd_reg;
};
