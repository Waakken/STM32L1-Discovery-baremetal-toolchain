#include "clocks.hpp"
#include "dma.hpp"
#include "gpio.hpp"
#include "lcd.hpp"
#include "reg_access.hpp"
#include "reg_defs.hpp"
#include "x86.hpp"

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

#define FULL_32 (0xffffffff)

// forward declarations
void blink_led(int count);

// definitions
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
    GPIO gpio;
    Clocks clocks;

    for (int i = 0; i < count; i++) {
        gpio.turn_on_led();
        clocks.delay(3000);
        // cpu_busy_loop_1_second();
        gpio.turn_off_led();
        clocks.delay(3000);
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
#ifdef __arm__
    while (1)
        ;
#endif
}

void test_sram()
{
    Lcd lcd;
    REG *sram_ptr = get_sram();
    REG sram_size = get_sram_size();

    for (REG i = 40; i < sram_size / 4; i++) {
        // for (REG i = 0; i < 10; i++) {
        sram_ptr[i] = 0x12345678;
        if (sram_ptr[i] != 0x12345678) {
            while (1)
                ;
        }
        // const char *str = lcd.int_to_str(i);
        lcd.reset();
        lcd.write_int_to_ram_buf(i);
        lcd.commit();
        // cpu_busy_loop_1_second();
    }
}

void demo_alphabets()
{
    // lcd.display_alphabet_in_location('x', 1);
    // lcd.commit();
    // arm_inf_loop();

    Lcd lcd;
    for (int i = 'a'; i < 'z'; i++) {
        lcd.display_alphabet_in_location(i, 1);
        lcd.commit();
        cpu_busy_loop_1_second();
        lcd.reset();
    }
    arm_inf_loop();
}

void demo_timer()
{
    Lcd lcd;
    Clocks clocks;

    unsigned single_tick_dur_ns_u = 1000000000 / 2097000;
    unsigned prescaler_1ms_per_tick = 1000000 / single_tick_dur_ns_u;
    printf_x86("Single tick: %u, prescaler_1ms_per_tick: %u\n",
               single_tick_dur_ns_u, prescaler_1ms_per_tick);

    display_int_on_lcd_for_one_second(single_tick_dur_ns_u);
    display_int_on_lcd_for_one_second(prescaler_1ms_per_tick);
    display_int_on_lcd_for_one_second(LOOPS_FOR_1_SEC_BUSY_LOOP);

    clocks.start_timer(prescaler_1ms_per_tick);
    while (1) {
        lcd.reset();
        lcd.write_int_to_ram_buf(get_tim2()->cnt);
        lcd.commit();
        // cpu_busy_loop_1_second();
        cpu_busy_loop_1_ms();
    }
    arm_inf_loop();
}

int main()
{
    Lcd lcd;
    GPIO gpio;
    Clocks clocks;
    Dma dma;

    redirect_pointers_in_x86();
    printf_x86("Initializing\n");
    // Initialization
    clocks.init_gpio_clocks();

    gpio.set_gpio_moder_to_af();
    gpio.set_gpio_af_modes();
    clocks.init_clocks_for_lcd();
    clocks.init_dma_clocks();
    lcd.init_lcd();
    lcd.reset();

    printf_x86("Program starts\n");
#ifdef __x86_64
    return 0;
#endif

    // test_sram();
    // demo_alphabets();
    // demo_timer();

    // Use following lines for manually scanning pixels through
    // lcd.set_ram_buf(4, FULL_32);
    // int bit_shift = 0;
    // while(1) {
    //     lcd.reset();
    //     lcd.write_int_to_ram_buf(bit_shift);
    //     lcd.commit();
    //     cpu_busy_loop_1_second();
    //     cpu_busy_loop_1_second();

    //     lcd.reset();
    //     lcd.set_ram_buf(4, 0xf << bit_shift);
    //     lcd.commit();
    //     cpu_busy_loop_1_second();
    //     cpu_busy_loop_1_second();
    //     bit_shift += 4;
    //     if (bit_shift > 28)
    //         bit_shift = 0;
    // }
    // lcd.set_ram_buf(4, 0xf << 24);

    // lcd.set_ram_buf_bit(4, 27);

    // lcd.commit();
    // arm_inf_loop();

    /*
0-1 -> 0
2-3 -> 1
4-5 -> 2
6-7 -> 3
    */

    // #ifndef __x86_64
    //     while (1)
    //         ;
    // #endif

    return 0;
}
