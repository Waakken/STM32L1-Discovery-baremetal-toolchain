#include "reg_defs.h"
#include "reg_addr.h"

struct gpio *gpioa = (struct gpio *)GPIOA;
struct gpio *gpiob = (struct gpio *)GPIOB;
struct gpio *gpioc = (struct gpio *)GPIOC;
struct gpio *gpiod = (struct gpio *)GPIOD;
struct gpio *gpioe = (struct gpio *)GPIOE;
struct rcc *rcc = (struct rcc *)RCC;
struct tim2 *tim2 = (struct tim2 *)TIM2;
struct rtc *rtc = (struct rtc *)RTC;
struct lcd *lcd = (struct lcd *)LCD;
struct pwr *pwr = (struct pwr *)PWR;

REG ram_buf[RAM_BUFS];
int ram_pixel_idx = 0;
int ram_buf_idx = 0;

#define SET_GPIO_MODER_BIT(reg, nth, val) (reg |= (val << (nth * 2)))
#define SET_GPIO_AFRL_BIT(reg, nth, val) (reg |= (val << (nth * 4)))
#define SET_GPIO_AFRH_BIT(reg, nth, val) (reg |= (val << ((nth - 8) * 4)))
#define SET_NTH_BIT(reg, nth) ((reg) |= (1 << (nth)))
#define FULL_32 (0xffffffff)

struct lcd_pixel {
    int com;
    int seg;
};

// LCD pixel mappings
struct lcd_pixel pixels_for_digit_low[6][8] =
    {
        //            A        B        C        D        E        F        G        H
        /* 0 */ {{1, 40}, {0, 40}, {1,  1}, {1,  0}, {0,  0}, {1, 41}, {0, 41}, {2, 13}},
        /* 1 */ {{1, 26}, {0, 26}, {1,  7}, {1,  2}, {0,  2}, {1, 27}, {0, 27}, {2, 13}},
        /* 2 */ {{1, 24}, {0, 24}, {1,  9}, {1,  8}, {0,  8}, {1, 25}, {0, 25}, {2, 13}},
        /* 3 */ {{1, 20}, {0, 20}, {1, 11}, {1, 10}, {0, 10}, {1, 21}, {0, 21}, {2, 13}},
        /* 4 */ {{1, 18}, {0, 18}, {1, 13}, {1, 12}, {0, 12}, {1, 19}, {0, 19}, {2, 13}},
        /* 5 */ {{1, 17}, {0, 17}, {1, 15}, {1, 14}, {0, 14}, {1, 16}, {0, 16}, {2, 13}}
    };

struct lcd_pixel pixels_for_digit_high[6][9] =
    {
        //            I        J        K        L        M        N        O        P        Q
        /* 0 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0,  1}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 1 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0,  7}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 2 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0,  9}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 3 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0, 11}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 4 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0, 13}, {2, 13}, {2, 13}, {2, 13}, {2, 13}},
        /* 5 */ {{2, 13}, {2, 13}, {2, 13}, {2, 13}, {0, 15}, {2, 13}, {2, 13}, {2, 13}, {2, 13}}
    };

// Digit pixel mappings
#define DIGITS 10
#define PIXELS_PER_DIGIT 14
const char digit_pixel_mappings[DIGITS][PIXELS_PER_DIGIT] =
    {
        /* 0 */ {"ABCDEFQQQQQQQQ"},
        /* 1 */ {"BCQQQQQQQQQQQQ"},
        /* 2 */ {"ABDEMGQQQQQQQQ"},
        /* 3 */ {"ABCDGMQQQQQQQQ"},
        /* 4 */ {"BCFGMQQQQQQQQQ"},
        /* 5 */ {"ACDFGMQQQQQQQQ"},
        /* 6 */ {"AFDGMCEQQQQQQQ"},
        /* 7 */ {"ABCQQQQQQQQQQQ"},
        /* 8 */ {"ABCDGMFEQQQQQQ"},
        /* 9 */ {"AFDGMCBQQQQQQQ"}
    };

// forward declarations
void blink_led(int count);
void show_digit_in_five(int digit);
void display_pixel(struct lcd_pixel);
void display_string(const char *str);
struct lcd_pixel map_pixel_alphabet(int digit, int alphabet);
void display_digit_in_location(int digit, int location);

// definitions
void display_pixel(struct lcd_pixel pix)
{
    int ram_buf_idx = pix.com * 2;
    int segment = pix.seg;
    if (segment > 31) {
        segment -= 32;
        ram_buf_idx++;
    }
    SET_NTH_BIT(ram_buf[ram_buf_idx], segment);
}

struct lcd_pixel map_pixel_alphabet(int digit, int alphabet)
{
    /*
     * args:
     *   -digit = Digit location on LCD display (0-5)
     *   -segment/pixel = Pixel to display (A-Q)
     */

    // TODO: Add support for checking arguments
    if (alphabet > 'H') {
        int segment = alphabet - 'I';
        return pixels_for_digit_high[digit][segment];
    } else {
        int segment = alphabet - 'A';
        return pixels_for_digit_low[digit][segment];
    }
}

int strlen(const char *str) {
    // TODO: Test
    /* int i = 0; */
    /* for (; str[i]; i++); */
    /* return i; */
    return PIXELS_PER_DIGIT;
}

void display_string(const char *str)
{
    // TODO: Use strlen to check input
    for (int i = 0; i < 6; i++) {
        int digit_idx = str[i] - '0';
        display_digit_in_location(digit_idx, i);
    }
}

void fill_ram_buf()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        ram_buf[i] = 0xffffffff;
    }
}

void write_next_pixel()
{
    ram_buf[ram_buf_idx] |= (1 << ram_pixel_idx);
    ram_pixel_idx++;
    if (ram_pixel_idx > 31){
        ram_pixel_idx = 0;
        ram_buf_idx++;
        blink_led(3);
    }
}

void write_full_buf()
{
    ram_buf[ram_buf_idx] = 0xffffffff;
    ram_buf_idx++;
    if (ram_buf_idx > RAM_BUFS)
        ram_buf_idx = 0;
}

void zero_ram_buf()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        ram_buf[i] = 0;
    }
}

void commit_lcd_ram_buf()
{
    // TODO: Need to wait and see that it's safe to write

    for (int i = 0; i < RAM_BUFS; i++) {
        lcd->ram[i] = ram_buf[i];
    }

    // Update display request
    lcd->sr |= (1 << 2);
}

void show_empty_screen()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        lcd->ram[i] = 0;
    }

    // Update display request
    lcd->sr |= (1 << 2);

    /* while(1); */
}

void zero_lcd_bits()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        lcd->ram[i] = 0;
    }

    // Update display request
    lcd->sr |= (1 << 2);

    /* while(1); */
}

void init_lcd()
{
    // bias 1/2
    //lcd->cr |=  (1 << 5);

    // bias 1/3
    lcd->cr |=  (1 << 6);

    // bias 1/4
    // = 0

    // duty 1/2
    //lcd->cr |= (1 << 2);

    // duty 1/3
    //lcd->cr |= (1 << 3);

    // duty 1/4
    lcd->cr |= (3 << 2);

    // duty 1/8
    //lcd->cr |= (1 << 4);

    // contrast 111
    /* lcd->fcr |=  (3 << 10); */
    lcd->fcr |=  (7 << 10);

    // pulse width
    /* lcd->fcr |= (7 << 4); */
    lcd->fcr |= (3 << 4);

    // disable mux
    lcd->cr &= ~(1 << 7);

    // use internal voltage source
    lcd->cr &= ~1;

    // enable
    lcd->cr |= 1;

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
}

void init_clocks_for_lcd()
{
    // Power interface clock enable
    rcc->apb1enr |= (1 << 28);

    // pwr->cr Looks OK!
    // Disable backup write protection
    pwr->cr |=  (1 << 8);

    // rcc->csr Looks OK!
    // enable LSI
    rcc->csr |= 1;

    // use LCI as LCD clock
    rcc->csr |= (1 << 17);

    // enable RTC clock
    rcc->csr |= (1 << 22);

    // enable LCD clock
    rcc->apb1enr |= (1 << 9);
}

void init_gpio_clocks() {
    rcc->ahbenr |= 0x1f;
}

void set_gpio_moder_to_af()
{
    char af = 2;

    SET_GPIO_MODER_BIT(gpioa->moder, 1, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 2, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 3, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 6, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 7, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 8, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 9, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 10, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 15, af);

    SET_GPIO_MODER_BIT(gpiob->moder, 0, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 1, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 3, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 4, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 5, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(gpiob->moder, i, af);

    for (char i = 0; i < 13; i++)
        SET_GPIO_MODER_BIT(gpioc->moder, i, af);

    SET_GPIO_MODER_BIT(gpiod->moder, 2, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(gpiod->moder, i, af);

    for (char i = 0; i < 5; i++)
        SET_GPIO_MODER_BIT(gpioe->moder, i, af);
}

void set_gpio_af_modes()
{
    // Set to AF 11

    char af_11 = 11;

    SET_GPIO_AFRL_BIT(gpioa->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 2, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 6, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 7, af_11);

    SET_GPIO_AFRH_BIT(gpioa->afrh, 8, af_11);
    SET_GPIO_AFRH_BIT(gpioa->afrh, 9, af_11);
    SET_GPIO_AFRH_BIT(gpioa->afrh, 10, af_11);
    SET_GPIO_AFRH_BIT(gpioa->afrh, 15, af_11);

    SET_GPIO_AFRL_BIT(gpiob->afrl, 0, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 4, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 5, af_11);
    for (char i = 8; i < 16; i++)
        SET_GPIO_AFRH_BIT(gpiob->afrh, i, af_11);

    for (char i = 0; i < 8; i++)
        SET_GPIO_AFRL_BIT(gpioc->afrl, i, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(gpioc->afrh, i, af_11);

    SET_GPIO_AFRL_BIT(gpiod->afrl, 2, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(gpiod->afrh, i, af_11);

    for (char i = 0; i < 4; i++)
        SET_GPIO_AFRL_BIT(gpioe->afrl, i, af_11);
}

void turn_on_led()
{
  gpioa->moder |= 1;
  gpioa->odr |= 1;
}

void turn_off_led()
{
  gpioa->odr &= ~1;
}

void delay(unsigned ms)
{
    /* enable tim2 */
    rcc->apb1enr |= 1;

    /* unset cr1 */
    tim2->cr1 &= ~1;
    tim2->sr = 0;

    /* clear counter */
    tim2->cnt = 0;

    /* set prescaler */
    /* 15 sec */

    /* 2 sec */
    // 0x200 = 512 ->
    // >>> 1 / 2097000 = 4.768717215069147e-07 thus 1 tick equals 476 ns
    // after prescaler 512 * 476 = 243.712us
    // 2000 * psc = 486.000ms
    // makes sense?
    tim2->psc = 0x200;

    /* arr = ms */
    tim2->arr = ms;

    /* enable tim2 */
    tim2->cr1 |= 1;

    while(!tim2->sr);
}

int delay_hack() {
    for (int i = 0; i < 400000; i++)
        asm("nop");
}

void blink_led(int count) {
    for (int i = 0; i < count; i++) {
        show_digit_in_five(i);
        turn_on_led();
        delay(3000);
        //delay_hack();
        turn_off_led();
        delay(3000);
        //delay_hack();
    }
}

void show_number_two_in_six() {
    SET_NTH_BIT(ram_buf[0], 14);
    SET_NTH_BIT(ram_buf[0], 15);
    SET_NTH_BIT(ram_buf[0], 16);
    SET_NTH_BIT(ram_buf[0], 17);

    SET_NTH_BIT(ram_buf[2], 14);
    SET_NTH_BIT(ram_buf[2], 17);
}

/***** 4 *****/

void show_number_zero_in_four() {
    struct lcd_pixel pix = map_pixel_alphabet(3, 'A');
    display_pixel(pix);
    pix = map_pixel_alphabet(3, 'B');
    display_pixel(pix);
    pix = map_pixel_alphabet(3, 'C');
    display_pixel(pix);
    pix = map_pixel_alphabet(3, 'D');
    display_pixel(pix);
    pix = map_pixel_alphabet(3, 'E');
    display_pixel(pix);
    pix = map_pixel_alphabet(3, 'F');
    display_pixel(pix);
}

void show_number_one_in_four() {
    SET_NTH_BIT(ram_buf[0], 20); // 4B
    SET_NTH_BIT(ram_buf[2], 11); // 4C
}

void show_number_two_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[0], 20); // 4B
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
    SET_NTH_BIT(ram_buf[0], 10); // 4E
    SET_NTH_BIT(ram_buf[2], 10); // 4D
}

void show_number_three_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[0], 20); // 4B
    SET_NTH_BIT(ram_buf[2], 11); // 4C
    SET_NTH_BIT(ram_buf[2], 10); // 4D
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
}

void show_number_four_in_four() {
    SET_NTH_BIT(ram_buf[0], 20); // 4B
    SET_NTH_BIT(ram_buf[2], 11); // 4C
    SET_NTH_BIT(ram_buf[2], 21); // 4F
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
}

void show_number_five_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
    SET_NTH_BIT(ram_buf[2], 10); // 4D
    SET_NTH_BIT(ram_buf[2], 21); // 4F
    SET_NTH_BIT(ram_buf[2], 11); // 4C
}

void show_number_six_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[2], 21); // 4F
    SET_NTH_BIT(ram_buf[2], 10); // 4D
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
    SET_NTH_BIT(ram_buf[2], 11); // 4C
    SET_NTH_BIT(ram_buf[0], 10); // 4E
}

void show_number_seven_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[0], 20); // 4B
    SET_NTH_BIT(ram_buf[2], 11); // 4C
}

void show_number_eight_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[0], 20); // 4B
    SET_NTH_BIT(ram_buf[2], 11); // 4C
    SET_NTH_BIT(ram_buf[2], 10); // 4D
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
    SET_NTH_BIT(ram_buf[2], 21); // 4F
    SET_NTH_BIT(ram_buf[0], 10); // 4E
}

void show_number_nine_in_four() {
    SET_NTH_BIT(ram_buf[2], 20); // 4A
    SET_NTH_BIT(ram_buf[2], 21); // 4F
    SET_NTH_BIT(ram_buf[2], 10); // 4D
    SET_NTH_BIT(ram_buf[0], 21); // 4G
    SET_NTH_BIT(ram_buf[0], 11); // 4M
    SET_NTH_BIT(ram_buf[2], 11); // 4C
    SET_NTH_BIT(ram_buf[0], 20); // 4B
}

/***** 5 *****/
void show_number_zero_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[2], 19); // 5F
    SET_NTH_BIT(ram_buf[2], 12); // 5D
    SET_NTH_BIT(ram_buf[2], 13); // 5C
    SET_NTH_BIT(ram_buf[0], 18); // 5B
    SET_NTH_BIT(ram_buf[0], 12); // 5E
}

void show_number_one_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[2], 13); // 5C
}

void show_number_two_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[0], 18); // 5B
    SET_NTH_BIT(ram_buf[0], 19); // 5G
    SET_NTH_BIT(ram_buf[0], 13); // 5M
    SET_NTH_BIT(ram_buf[0], 12); // 5E
    SET_NTH_BIT(ram_buf[2], 12); // 5D
}

void show_number_three_in_five() {
    SET_NTH_BIT(ram_buf[2], 18);
    SET_NTH_BIT(ram_buf[0], 18);
    SET_NTH_BIT(ram_buf[2], 13);
    SET_NTH_BIT(ram_buf[2], 12);
    SET_NTH_BIT(ram_buf[0], 19);
    SET_NTH_BIT(ram_buf[0], 13);
}

void show_number_four_in_five() {
    SET_NTH_BIT(ram_buf[0], 18); // 5B
    SET_NTH_BIT(ram_buf[2], 13); // 5C
    SET_NTH_BIT(ram_buf[2], 19); // 5F
    SET_NTH_BIT(ram_buf[0], 19); // 5G
    SET_NTH_BIT(ram_buf[0], 13); // 5M
}

void show_number_five_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[0], 19); // 5G
    SET_NTH_BIT(ram_buf[0], 13); // 5M
    SET_NTH_BIT(ram_buf[2], 12); // 5D
    SET_NTH_BIT(ram_buf[2], 19); // 5F
    SET_NTH_BIT(ram_buf[2], 13); // 5C
}

void show_number_six_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[2], 19); // 5F
    SET_NTH_BIT(ram_buf[2], 12); // 5D
    SET_NTH_BIT(ram_buf[0], 19); // 5G
    SET_NTH_BIT(ram_buf[0], 13); // 5M
    SET_NTH_BIT(ram_buf[2], 13); // 5C
    SET_NTH_BIT(ram_buf[0], 12); // 5E
}

void show_number_seven_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[0], 18); // 5B
    SET_NTH_BIT(ram_buf[2], 13); // 5C
}

void show_number_eight_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[2], 19); // 5F
    SET_NTH_BIT(ram_buf[2], 12); // 5D
    SET_NTH_BIT(ram_buf[0], 19); // 5G
    SET_NTH_BIT(ram_buf[0], 13); // 5M
    SET_NTH_BIT(ram_buf[2], 13); // 5C
    SET_NTH_BIT(ram_buf[0], 12); // 5E
    SET_NTH_BIT(ram_buf[0], 18); // 5B
}

void show_number_nine_in_five() {
    SET_NTH_BIT(ram_buf[2], 18); // 5A
    SET_NTH_BIT(ram_buf[2], 19); // 5F
    SET_NTH_BIT(ram_buf[2], 12); // 5D
    SET_NTH_BIT(ram_buf[0], 19); // 5G
    SET_NTH_BIT(ram_buf[0], 13); // 5M
    SET_NTH_BIT(ram_buf[2], 13); // 5C
    SET_NTH_BIT(ram_buf[0], 18); // 5B
}

void show_digit_in_five(int digit) {
        zero_ram_buf();
        switch(digit) {
            case 0:
                show_number_zero_in_five();
                break;
            case 1:
                show_number_one_in_five();
                break;
            case 2:
                show_number_two_in_five();
                break;
            case 3:
                show_number_three_in_five();
                break;
            case 4:
                show_number_four_in_five();
                break;
            case 5:
                show_number_five_in_five();
                break;
            case 6:
                show_number_six_in_five();
                break;
            case 7:
                show_number_seven_in_five();
                break;
            case 8:
                show_number_eight_in_five();
                break;
            case 9:
                show_number_nine_in_five();
                break;
            default:
                fill_ram_buf();
                break;
        }
        commit_lcd_ram_buf();
}

void demo_loop_numbers() {
    int i = 0;
    while (1) {
        show_digit_in_five(i);
        delay(4000);
        i++;
        i %= 10;
    }
}

void display_digit_in_location(int digit, int location)
{
    int digit_idx = digit;
    const char *str_for_digit = digit_pixel_mappings[digit_idx];
    struct lcd_pixel pix;
    for (int j = 0; j < PIXELS_PER_DIGIT; j++) {
        char segment = str_for_digit[j];
        pix = map_pixel_alphabet(location, segment);
        display_pixel(pix);
    }
}

int main() {
    // Initialization
    asm("nop");
    asm("nop");
    init_gpio_clocks();

    set_gpio_moder_to_af();
    set_gpio_af_modes();
    init_clocks_for_lcd();
    asm("nop");
    asm("nop");
    asm("nop");
    init_lcd();
    zero_ram_buf();

    // Program starts
    //blink_led(10);

    //demo_loop_numbers();
    // DEMO
    /* show_number_zero_in_four(); */
    /* show_number_eight_in_five(); */
    display_string("123456");
    //display_string("888888");


    /* fill_ram_buf(); */
    //ram_buf[4] = FULL_32;
    //ram_buf[4] |= 0xf << 12;
    //SET_NTH_BIT(ram_buf[4], 13);
    commit_lcd_ram_buf();
    while(1);

    while (1) {
        zero_ram_buf();
        write_full_buf();
        commit_lcd_ram_buf();
        delay(4000);
        /* delay_hack(); */

        /* show_empty_screen(); */
        /* if (ram_buf_idx >= RAM_BUFS) */
        /*     break; */
    }
    /* delay(4000); */
    /* blink_led(5); */
    while(1);

    return 0;
}
