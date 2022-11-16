#include "lcd.hpp"
#include "x86.hpp"

// TODO: Use definition instead of magic number

/*
Digits and characters are expressed as letters describing segments: A-Q.

   A
 H J K
F     B
  G M
E     C
 Q P N
   D
*/

// LCD pixel mappings
// 2, 13 is the 'default': not yet implemented.
// clang-format off
constexpr struct lcd_pixel pixels_for_digit_low[6][8] =
    {
        //            A        B        C        D        E        F        G        H
        /* 0 */ {{1, 40}, {0, 40}, {1,  1}, {1,  0}, {0,  0}, {1, 41}, {0, 41}, {2, 13}},
        /* 1 */ {{1, 26}, {0, 26}, {1,  7}, {1,  2}, {0,  2}, {1, 27}, {0, 27}, {3, 27}},
        /* 2 */ {{1, 24}, {0, 24}, {1,  9}, {1,  8}, {0,  8}, {1, 25}, {0, 25}, {2, 13}},
        /* 3 */ {{1, 20}, {0, 20}, {1, 11}, {1, 10}, {0, 10}, {1, 21}, {0, 21}, {2, 13}},
        /* 4 */ {{1, 18}, {0, 18}, {1, 13}, {1, 12}, {0, 12}, {1, 19}, {0, 19}, {2, 13}},
        /* 5 */ {{1, 17}, {0, 17}, {1, 15}, {1, 14}, {0, 14}, {1, 16}, {0, 16}, {2, 13}}
    };

constexpr struct lcd_pixel pixels_for_digit_high[6][9] =
    {
        //            I        J        K        L        M        N        O        P        Q
        /* 0 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0,  1}, {2, 13}, {2, 13}, {2,  0}, {2, 13}},
        /* 1 */ {{2, 13}, {3, 26}, {2, 26}, {2, 13}, {0,  7}, {3,  2}, {2, 13}, {2,  2}, {2, 27}},
        /* 2 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0,  9}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 3 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0, 11}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 4 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0, 13}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 5 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0, 15}, {2, 13}, {2, 13}, {2, 13}, {2, 13}}
    };
// clang-format on

// Digit pixel mappings
#define DIGITS 10
#define PIXELS_PER_DIGIT 15
constexpr const char digit_pixel_mappings[DIGITS][PIXELS_PER_DIGIT] = {
    /* 0 */ {"ABCDEF"},
    /* 1 */ {"BC"},
    /* 2 */ {"ABDEMG"},
    /* 3 */ {"ABCDGM"},
    /* 4 */ {"BCFGM"},
    /* 5 */ {"ACDFGM"},
    /* 6 */ {"AFDGMCE"},
    /* 7 */ {"ABC"},
    /* 8 */ {"ABCDGMFE"},
    /* 9 */ {"AFDGMCB"}};

// Alphabet pixel mappings
constexpr int ALPHABETS{26};

constexpr char alphabet_pixel_mappings[ALPHABETS][PIXELS_PER_DIGIT] = {
    /* a */ {"ABCEFGM"},
    /* b */ {"CDEFGM"},
    /* c */ {"AFED"},
    /* d */ {"BCDEGM"},
    /* e */ {"AFEDGM"},
    /* f */ {"AFEGM"},
    /* g */ {"AFDGMCE"},
    /* h */ {"BCEFGM"},
    /* i */ {"FE"},
    /* j */ {"BCDE"},

    /* k */ {"EFGKN"},
    /* l */ {"FED"},
    /* m */ {"EFHKBC"},
    /* n */ {"EFHNBC"},
    /* o */ {"ABCDEF"},
    /* p */ {"EFABGM"},
    /* q */ {"ABCFGM"},
    /* r */ {"EFABGMN"},
    /* s */ {"ACDFGM"},
    /* t */ {"AJP"},

    /* u */ {"FEDBC"},
    /* v */ {"HNBC"},
    /* w */ {"EFQJNCB"},
    /* x */ {"HKQN"},
    /* y */ {"HKP"},
    /* z */ {"AKQD"}};

void Lcd::display_digit_in_location(int digit, int location)
{
    if (digit > 9)
        return;

    const char *str_for_digit = digit_pixel_mappings[digit];
    printf_x86("%s digit: %u, location: %u, str_for_digit: %s\n", __func__,
               digit, location, str_for_digit);
    struct lcd_pixel pix;
    for (int j = 0; j < my_strlen(str_for_digit); j++) {
        char segment = str_for_digit[j];
        pix = map_pixel_alphabet(location, segment);
        display_pixel(pix);
    }
}

void Lcd::display_alphabet_in_location(int alphabet, int location)
{
    if (alphabet > 'z' || alphabet < 'a') {
        printf_x86("%s error: bad args, alphabet: %u, location: %d", __func__,
                   alphabet, location);
        return;
    }
    alphabet -= 'a';

    const char *str_for_digit = alphabet_pixel_mappings[alphabet];
    printf_x86("%s alphabet: %u, location: %u, str_for_digit: %s\n", __func__,
               alphabet, location, str_for_digit);
    struct lcd_pixel pix;
    for (int j = 0; j < my_strlen(str_for_digit); j++) {
        char segment = str_for_digit[j];
        pix = map_pixel_alphabet(location, segment);
        display_pixel(pix);
    }
}

struct lcd_pixel Lcd::map_pixel_alphabet(int digit, int segment_alphabet)
{
    /*
     * args:
     *   -digit = Digit location on LCD display (0-5)
     *   -segment/pixel = Pixel to display (A-Q)
     */

    // TODO: Add support for checking arguments
    if (segment_alphabet > 'H') {
        int segment = segment_alphabet - 'I';
        return pixels_for_digit_high[digit][segment];
    } else {
        int segment = segment_alphabet - 'A';
        return pixels_for_digit_low[digit][segment];
    }
}

void Lcd::set_ram_buf_bit(int ram_buf_idx, int bit_idx)
{
    // ram_buf[4] = FULL_32;
    // ram_buf[4] |= 0xf << 12;
    if (ram_buf_idx > RAM_BUFS)
        return;

    SET_NTH_BIT(ram_buf[ram_buf_idx], bit_idx);
}

void Lcd::set_ram_buf(int ram_buf_idx, REG val)
{
    // ram_buf[4] = FULL_32;
    // ram_buf[4] |= 0xf << 12;
    // SET_NTH_BIT(ram_buf[4], 13);
    if (ram_buf_idx > RAM_BUFS)
        return;

    ram_buf[ram_buf_idx] = val;
}

void Lcd::fill_ram_buf()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        ram_buf[i] = 0xffffffff;
    }
}

void Lcd::write_next_pixel()
{
    ram_buf[ram_buf_idx] |= (1 << ram_pixel_idx);
    ram_pixel_idx++;
    if (ram_pixel_idx > 31) {
        ram_pixel_idx = 0;
        ram_buf_idx++;
        /* blink_led(3); */
    }
}

void Lcd::write_full_buf()
{
    ram_buf[ram_buf_idx] = 0xffffffff;
    ram_buf_idx++;
    if (ram_buf_idx > RAM_BUFS)
        ram_buf_idx = 0;
}

void Lcd::reset()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        ram_buf[i] = 0;
    }
}

void Lcd::commit()
{
    // TODO: Need to wait and see that it's safe to write

    for (int i = 0; i < RAM_BUFS; i++) {
        lcd_reg->ram[i] = ram_buf[i];
    }

    // Update display request
    lcd_reg->sr |= (1 << 2);
}

void Lcd::display_pixel(struct lcd_pixel pix)
{
    int ram_buf_idx = pix.com * 2;
    int segment = pix.seg;
    if (segment > 31) {
        segment -= 32;
        ram_buf_idx++;
    }
    SET_NTH_BIT(ram_buf[ram_buf_idx], segment);
}

void Lcd::init_lcd()
{
    // bias 1/2
    // lcd->cr |=  (1 << 5);

    // bias 1/3
    lcd_reg->cr |= (1 << 6);

    // bias 1/4
    // = 0

    // duty 1/2
    // lcd->cr |= (1 << 2);

    // duty 1/3
    // lcd->cr |= (1 << 3);

    // duty 1/4
    lcd_reg->cr |= (3 << 2);

    // duty 1/8
    // lcd->cr |= (1 << 4);

    // contrast 111
    /* lcd->fcr |=  (3 << 10); */
    lcd_reg->fcr |= (7 << 10);

    // pulse width
    /* lcd->fcr |= (7 << 4); */
    lcd_reg->fcr |= (3 << 4);

    // disable mux
    lcd_reg->cr &= ~(1 << 7);

    // use internal voltage source
    lcd_reg->cr &= ~1;

    // enable
    lcd_reg->cr |= 1;
}

int Lcd::my_strlen(const volatile char *str)
{
    int i = 0;
    for (; str[i]; i++)
        ;
    return i;
}

void Lcd::write_int_to_ram_buf(int num)
{
    const char *str = int_to_str(num);
    write_string_to_ram_buf(str);
}

void Lcd::write_string_to_ram_buf(const char *str)
{
    printf_x86("Displaying: %s\n", str);
    for (int i = 0; i < my_strlen(str); i++) {
        int digit_idx = str[i] - '0';
        display_digit_in_location(digit_idx, i);
    }
}

const char *Lcd::int_to_str(int num)
{
    // Reinitialize digit_str buffer
    for (int i = 0; i < 6; i++)
        digit_str[i] = 0;

    // int digits = count_digits(num);

    int pow_10 = 10;

    // First digit:
    int cur_digit = num % pow_10;
    digit_str[5] = cur_digit + '0';

    pow_10 *= 10;

    int digit_str_index = 4;
    for (int i = 1; i < 6; i++)
    // for (int i = 0; i < digits; i++)
    {
        // TODO: Add support for first digit
        int mod = num % pow_10;
        cur_digit = mod / (pow_10 / 10);
        digit_str[digit_str_index] = cur_digit + '0';
        // digit_str[0] = num + '0';
        int digits = 0;
        printf_x86("%s digits: %u num: %u loop: %u pow 10: %u, cur "
                   "digit: %u, mod: %u, "
                   "str: %s\n",
                   __func__, digits, num, i, pow_10, cur_digit, mod, digit_str);
        pow_10 *= 10;
        digit_str_index--;
    }
    return digit_str;
}
