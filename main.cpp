#include "lcd.hpp"
#include "reg_access.hpp"
#include "reg_defs.hpp"

#ifdef __x86_64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#define MSI_CLOCK_DEFAULT_FREQ 2097000
// With -O1:
#define LOOPS_FOR_1_SEC_BUSY_LOOP (MSI_CLOCK_DEFAULT_FREQ / 6)
// With -O0:
//#define LOOPS_FOR_1_SEC_BUSY_LOOP (MSI_CLOCK_DEFAULT_FREQ / 15)

#define SET_GPIO_MODER_BIT(reg, nth, val) (reg |= (val << (nth * 2)))
#define SET_GPIO_AFRL_BIT(reg, nth, val) (reg |= (val << (nth * 4)))
#define SET_GPIO_AFRH_BIT(reg, nth, val) (reg |= (val << ((nth - 8) * 4)))
#define FULL_32 (0xffffffff)

// forward declarations
void blink_led(int count);
void show_digit_in_five(int digit);
/* void display_pixel(struct lcd_pixel); */
void write_string_to_ram_buf(const char *str);
struct lcd_pixel map_pixel_alphabet(int digit, int alphabet);
void display_digit_in_location(int digit, int location);
const char *int_to_str(int num);
void printf_x86(const char *fmt, ...);

// definitions
void init_clocks_for_lcd()
{
    // Power interface clock enable
    get_rcc()->apb1enr |= (1 << 28);

    // pwr->cr Looks OK!
    // Disable backup write protection
    get_pwr()->cr |= (1 << 8);

    // rcc->csr Looks OK!
    // enable LSI
    get_rcc()->csr |= 1;

    // use LCI as LCD clock
    get_rcc()->csr |= (1 << 17);

    // enable RTC clock
    get_rcc()->csr |= (1 << 22);

    // enable LCD clock
    get_rcc()->apb1enr |= (1 << 9);
}

void init_gpio_clocks()
{
    // GPIO A-H EN
    get_rcc()->ahbenr |= 0x1f;
}

void set_gpio_moder_to_af()
{
    char af = 2;

    SET_GPIO_MODER_BIT(get_gpioa()->moder, 1, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 2, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 3, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 6, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 7, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 8, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 9, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 10, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 15, af);

    SET_GPIO_MODER_BIT(get_gpiob()->moder, 0, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 1, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 3, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 4, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 5, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(get_gpiob()->moder, i, af);

    for (char i = 0; i < 13; i++)
        SET_GPIO_MODER_BIT(get_gpioc()->moder, i, af);

    SET_GPIO_MODER_BIT(get_gpiod()->moder, 2, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(get_gpiod()->moder, i, af);

    for (char i = 0; i < 5; i++)
        SET_GPIO_MODER_BIT(get_gpioe()->moder, i, af);
}

void set_gpio_af_modes()
{
    // Set to AF 11

    char af_11 = 11;

    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 2, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 6, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 7, af_11);

    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 8, af_11);
    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 9, af_11);
    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 10, af_11);
    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 15, af_11);

    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 0, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 4, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 5, af_11);
    for (char i = 8; i < 16; i++)
        SET_GPIO_AFRH_BIT(get_gpiob()->afrh, i, af_11);

    for (char i = 0; i < 8; i++)
        SET_GPIO_AFRL_BIT(get_gpioc()->afrl, i, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(get_gpioc()->afrh, i, af_11);

    SET_GPIO_AFRL_BIT(get_gpiod()->afrl, 2, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(get_gpiod()->afrh, i, af_11);

    for (char i = 0; i < 4; i++)
        SET_GPIO_AFRL_BIT(get_gpioe()->afrl, i, af_11);
}

void turn_on_led()
{
    get_gpioa()->moder |= 1;
    get_gpioa()->odr |= 1;
}

void turn_off_led() { get_gpioa()->odr &= ~1; }

void start_timer(unsigned prescaler)
{
    /* enable tim2 */
    get_rcc()->apb1enr |= 1;

    /* unset cr1 */
    get_tim2()->cr1 &= ~1;
    get_tim2()->sr = 0;

    /* clear counter */
    get_tim2()->cnt = 0;

    /*
      2.097 mhz clock
      >>> 1 / 2097000 = 4.768717215069147e-07 thus 1 tick equals 476 ns

      how to achieve: 1 tick = 1ms (1M ns)?

      476 * x = 1000000
            x = 1000000 / 476
            x = 2100
    */
    get_tim2()->psc = prescaler;

    /* enable tim2 */
    get_tim2()->cr1 |= 1;
}

void delay(unsigned ms)
{
    // TODO: This function doesn't work currently
    /* enable tim2 */
    get_rcc()->apb1enr |= 1;

    /* unset cr1 */
    get_tim2()->cr1 &= ~1;
    get_tim2()->sr = 0;

    /* clear counter */
    get_tim2()->cnt = 0;

    /* set prescaler */
    /* 15 sec */

    /* 2 sec */
    // 0x200 = 512 ->
    // >>> 1 / 2097000 = 4.768717215069147e-07 thus 1 tick equals 476 ns
    // after prescaler 512 * 476 = 243.712us
    // 2000 * psc = 486.000ms
    // makes sense?
    get_tim2()->psc = 0x200;

    /* arr = ms */
    get_tim2()->arr = ms;

    /* enable tim2 */
    get_tim2()->cr1 |= 1;

    while (!get_tim2()->sr)
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

void cpu_busy_loop_1_ms()
{
    for (int i = 0; i < LOOPS_FOR_1_SEC_BUSY_LOOP / 1000; i++) {
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

void cpu_busy_loop_10_loops()
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

int count_digits(int num)
{
    // TODO: Use definition instead of magic number
    int pow_10 = 10;
    int i = 1;
    for (; i < 7; i++) {
        printf_x86("%s num: %u loop: %u pow 10: %u\n", __func__, num, i,
                   pow_10);
        int div = num / pow_10;
        if (!div)
            break;
        pow_10 *= 10;
    }
    printf_x86("%s num: %u has %u digits\n", __func__, num, i);
    return i;
}

void display_int_on_lcd_for_one_second(unsigned long number)
{
    Lcd lcd;
    const char *str = lcd.int_to_str(number);
    lcd.reset();
    lcd.write_string_to_ram_buf(str);
    lcd.commit();
    cpu_busy_loop_1_second();
}

void arm_inf_loop()
{
#ifdef __aarch64
    while (1)
        ;
#endif
}

int main()
{
    Lcd lcd;
    redirect_pointers_in_x86();
    printf_x86("Initializing\n");
    // Initialization
    init_gpio_clocks();

    set_gpio_moder_to_af();
    set_gpio_af_modes();
    init_clocks_for_lcd();
    lcd.init_lcd();
    lcd.reset();

    printf_x86("Program starts\n");
    // Use following lines for manually scanning pixels through
    // ram_buf[4] = FULL_32;
    // ram_buf[4] |= 0xf << 12;
    // SET_NTH_BIT(ram_buf[4], 13);
    // lcd.set_ram_buf(4, FULL_32);
    lcd.set_ram_buf(4, 0xf << 4);
    // lcd.set_ram_buf_bit(4, 2);
    lcd.commit();

    arm_inf_loop();

#ifdef __x86_64
    return 0;
#endif

    for (int i = 'a'; i < 'z'; i++) {
        lcd.display_alphabet_in_location(i, 1);
        lcd.commit();
        cpu_busy_loop_1_second();
        lcd.reset();
    }

    arm_inf_loop();

    unsigned single_tick_dur_ns_u = 1000000000 / 2097000;
    unsigned prescaler_1ms_per_tick = 1000000 / single_tick_dur_ns_u;
    printf_x86("Single tick: %u, prescaler_1ms_per_tick: %u\n",
               single_tick_dur_ns_u, prescaler_1ms_per_tick);

    display_int_on_lcd_for_one_second(single_tick_dur_ns_u);
    display_int_on_lcd_for_one_second(prescaler_1ms_per_tick);
    display_int_on_lcd_for_one_second(LOOPS_FOR_1_SEC_BUSY_LOOP);

    start_timer(prescaler_1ms_per_tick);
    while (1) {
        lcd.reset();
        lcd.write_int_to_ram_buf(get_tim2()->cnt);
        lcd.commit();
        cpu_busy_loop_1_second();
        /* cpu_busy_loop_1_ms(); */
    }
    arm_inf_loop();
    // #ifndef __x86_64
    //     while (1)
    //         ;
    // #endif

    return 0;
}
