#include "reg_addr.h"
#include "reg_defs.h"

#ifdef __x86_64
#include <stdio.h>
#include <stdlib.h>
#endif

#define MSI_CLOCK_DEFAULT_FREQ 2097000
// With -O1:
#define LOOPS_FOR_1_SEC_BUSY_LOOP (MSI_CLOCK_DEFAULT_FREQ / 6)
// With -O0:
//#define LOOPS_FOR_1_SEC_BUSY_LOOP (MSI_CLOCK_DEFAULT_FREQ / 15)

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

// CPU side copy of the LCD pixel buffer. Always copied fully to LCD memory
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
// clang-format on

// Digit pixel mappings
#define DIGITS 10
#define PIXELS_PER_DIGIT 15
const char digit_pixel_mappings[DIGITS][PIXELS_PER_DIGIT] = {
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

// forward declarations
void blink_led(int count);
void show_digit_in_five(int digit);
void display_pixel(struct lcd_pixel);
void write_string_to_ram_buf(const char *str);
struct lcd_pixel map_pixel_alphabet(int digit, int alphabet);
void display_digit_in_location(int digit, int location);
const char *int_to_str(int num);

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

int my_strlen(const char *str)
{
    int i = 0;
    for (; str[i]; i++)
        ;
    return i;
}

void write_int_to_ram_buf(int num)
{
    const char *str = int_to_str(num);
    write_string_to_ram_buf(str);
}

void write_string_to_ram_buf(const char *str)
{
#ifdef __x86_64
    printf("Displaying: %s\n", str);
#endif
    for (int i = 0; i < my_strlen(str); i++) {
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
    if (ram_pixel_idx > 31) {
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
    // lcd->cr |=  (1 << 5);

    // bias 1/3
    lcd->cr |= (1 << 6);

    // bias 1/4
    // = 0

    // duty 1/2
    // lcd->cr |= (1 << 2);

    // duty 1/3
    // lcd->cr |= (1 << 3);

    // duty 1/4
    lcd->cr |= (3 << 2);

    // duty 1/8
    // lcd->cr |= (1 << 4);

    // contrast 111
    /* lcd->fcr |=  (3 << 10); */
    lcd->fcr |= (7 << 10);

    // pulse width
    /* lcd->fcr |= (7 << 4); */
    lcd->fcr |= (3 << 4);

    // disable mux
    lcd->cr &= ~(1 << 7);

    // use internal voltage source
    lcd->cr &= ~1;

    // enable
    lcd->cr |= 1;
}

void init_clocks_for_lcd()
{
    // Power interface clock enable
    rcc->apb1enr |= (1 << 28);

    // pwr->cr Looks OK!
    // Disable backup write protection
    pwr->cr |= (1 << 8);

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

void init_gpio_clocks()
{
    // GPIO A-H EN
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

void turn_off_led() { gpioa->odr &= ~1; }

void start_timer(unsigned prescaler)
{
    /* enable tim2 */
    rcc->apb1enr |= 1;

    /* unset cr1 */
    tim2->cr1 &= ~1;
    tim2->sr = 0;

    /* clear counter */
    tim2->cnt = 0;

    /*
      2.097 mhz clock
      >>> 1 / 2097000 = 4.768717215069147e-07 thus 1 tick equals 476 ns

      how to achieve: 1 tick = 1ms (1M ns)?

      476 * x = 1000000
            x = 1000000 / 476
            x = 2100
    */
    tim2->psc = prescaler;

    /* enable tim2 */
    tim2->cr1 |= 1;
}

void delay(unsigned ms)
{
    // TODO: This function doesn't work currently
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

    while (!tim2->sr)
        ;
}

void cpu_busy_loop_1_second()
{
    // 5 instructions, 6 cycles per loop:
    //   nop * 3
    //   subs
    //   bne.n (2 cycles)
    for (int i = 0; i < LOOPS_FOR_1_SEC_BUSY_LOOP; i++) {
        asm("nop");
        asm("nop");
        asm("nop");
    }
    return;
}

int cpu_busy_loop_1_ms()
{
    for (int i = 0; i < LOOPS_FOR_1_SEC_BUSY_LOOP / 1000; i++) {
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

int cpu_busy_loop_10_loops()
{
    for (int i = 0; i < 10; i++) {
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

void blink_led(int count)
{
    for (int i = 0; i < count; i++) {
        turn_on_led();
        delay(3000);
        // cpu_busy_loop_1_second();
        turn_off_led();
        delay(3000);
        // cpu_busy_loop_1_second();
    }
}

void display_digit_in_location(int digit, int location)
{
    int digit_idx = digit;
    const char *str_for_digit = digit_pixel_mappings[digit_idx];
#ifdef __x86_64
    printf("%s digit: %u, location: %u, str_for_digit: %s\n", __func__, digit,
           location, str_for_digit);
#endif
    struct lcd_pixel pix;
    for (int j = 0; j < my_strlen(str_for_digit); j++) {
        char segment = str_for_digit[j];
        pix = map_pixel_alphabet(location, segment);
        display_pixel(pix);
    }
}

// TODO: Use definition instead of magic number
char digit_str[6] = {0, 0, 0, 0, 0, 0};

int count_digits(int num)
{
    // TODO: Use definition instead of magic number
    int pow_10 = 10;
    int i = 1;
    for (; i < 7; i++) {
#ifdef __x86_64
        printf("%s num: %u loop: %u pow 10: %u\n", __func__, num, i, pow_10);
#endif
        int div = num / pow_10;
        if (!div)
            break;
        pow_10 *= 10;
    }
#ifdef __x86_64
    printf("%s num: %u has %u digits\n", __func__, num, i);
#endif
    return i;
}

const char *int_to_str(int num)
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
#ifdef __x86_64
        int digits = 0;
        printf("%s digits: %u num: %u loop: %u pow 10: %u, cur "
               "digit: %u, mod: %u, "
               "str: %s\n",
               __func__, digits, num, i, pow_10, cur_digit, mod, digit_str);
#endif
        pow_10 *= 10;
        digit_str_index--;
    }
    return digit_str;
}

#ifdef __x86_64
void redirect_pointers_in_x86()
{
    // TODO: Allocate contiguous area, with size of 0x40023800 -
    // 0x40000000
    gpioa = malloc(sizeof(struct gpio));
    gpiob = malloc(sizeof(struct gpio));
    gpioc = malloc(sizeof(struct gpio));
    gpiod = malloc(sizeof(struct gpio));
    gpioe = malloc(sizeof(struct gpio));
    rcc = malloc(sizeof(struct rcc));
    tim2 = malloc(sizeof(struct tim2));
    rtc = malloc(sizeof(struct rtc));
    lcd = malloc(sizeof(struct lcd));
    pwr = malloc(sizeof(struct pwr));
}
#endif

void display_int_on_lcd_for_one_second(unsigned long number)
{
    const char *str = int_to_str(number);
    zero_ram_buf();
    write_string_to_ram_buf(str);
    commit_lcd_ram_buf();
    cpu_busy_loop_1_second();
}

int main()
{
#ifdef __x86_64
    printf("Redirecting pointers\n");
    redirect_pointers_in_x86();
    printf("Initializing\n");
#endif
    // Initialization
    init_gpio_clocks();

    set_gpio_moder_to_af();
    set_gpio_af_modes();
    init_clocks_for_lcd();
    init_lcd();
    zero_ram_buf();

#ifdef __x86_64
    printf("Program starts\n");
#endif
    // Use following lines for manually scanning pixels through
    // ram_buf[4] = FULL_32;
    // ram_buf[4] |= 0xf << 12;
    // SET_NTH_BIT(ram_buf[4], 13);

    unsigned single_tick_dur_ns_u = 1000000000 / 2097000;
    unsigned prescaler_1ms_per_tick = 1000000 / single_tick_dur_ns_u;
#ifdef __x86_64
    printf("Single tick: %u, prescaler_1ms_per_tick: %u\n",
           single_tick_dur_ns_u, prescaler_1ms_per_tick);
#endif

    display_int_on_lcd_for_one_second(single_tick_dur_ns_u);
    display_int_on_lcd_for_one_second(prescaler_1ms_per_tick);
    display_int_on_lcd_for_one_second(LOOPS_FOR_1_SEC_BUSY_LOOP);

#ifdef __x86_64
    return 0;
#endif

    start_timer(prescaler_1ms_per_tick);
    while (1) {
        zero_ram_buf();
        write_int_to_ram_buf(tim2->cnt);
        commit_lcd_ram_buf();
        /* cpu_busy_loop_1_second(); */
        cpu_busy_loop_1_ms();
    }
#ifndef __x86_64
    while (1)
        ;
#endif

    return 0;
}
