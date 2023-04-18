#include "clocks.hpp"
#include "dma.hpp"
#include "gpio.hpp"
#include "lcd.hpp"
#include "reg_access.hpp"
#include "reg_defs.hpp"
#include "singleton.hpp"
#include "uart.hpp"
#include "x86.hpp"

#ifdef __x86_64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif

static_assert(sizeof(int) == 4, "int assumed to be 4 bytes");
static_assert(sizeof(unsigned) == 4, "unsigned assumed to be 4 bytes");
#ifdef __x86_64
static_assert(sizeof(REG) == 8, "REG assumed to be 8 bytes in x86");
#else
static_assert(sizeof(REG) == 4, "REG assumed to be 4 bytes in arm");
#endif

// 2.097 MHz
#define MSI_CLOCK_DEFAULT_FREQ 2097000
// With -O1:
#define LOOPS_FOR_1_SEC_BUSY_LOOP (MSI_CLOCK_DEFAULT_FREQ / 6)
// With -O0:
// #define LOOPS_FOR_1_SEC_BUSY_LOOP (MSI_CLOCK_DEFAULT_FREQ / 15)

#define FULL_32 (0xffffffff)

char dma_demo_bss_buffer[0x1000];

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
    GPIO &gpio = GPIO::get();
    Clocks &clocks = Clocks::get();

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
    int pow_10 = 10;
    int i = 1;
    for (; i < 7; i++) {
        printf_x86("%s num: %u loop: %u pow 10: %u\n", __func__, num, i, pow_10);
        int div = num / pow_10;
        if (!div)
            break;
        pow_10 *= 10;
    }
    printf_x86("%s num: %u has %u digits\n", __func__, num, i);
    return i;
}

void display_str_on_lcd(const char *str)
{
    Lcd &lcd = Lcd::get();
    lcd.str_to_str(str);
    lcd.reset_ram_buf();
    lcd.write_string_to_ram_buf();
    lcd.commit();
}

void display_int_on_lcd(int number)
{
    Lcd &lcd = Lcd::get();
    lcd.int_to_str(number);
    lcd.reset_ram_buf();
    lcd.write_string_to_ram_buf();
    lcd.commit();
}

void display_hex_on_lcd(int number)
{
    Lcd &lcd = Lcd::get();
    lcd.hex_to_str(number);
    lcd.reset_ram_buf();
    lcd.write_string_to_ram_buf();
    lcd.commit();
}

void display_int_on_lcd_for_one_second(int number)
{
    display_int_on_lcd(number);
    cpu_busy_loop_1_second();
}

void display_hex_on_lcd_for_two_seconds(int number)
{
    display_hex_on_lcd(number);
    cpu_busy_loop_1_second();
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
    Lcd &lcd = Lcd::get();
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
        lcd.reset_ram_buf();
        lcd.write_int_to_ram_buf(i);
        lcd.commit();
        // cpu_busy_loop_1_second();
    }
}

void demo_hex_alphabets()
{
    Lcd &lcd = Lcd::get();
    const char *hex_alphabets[6] = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
    const char *cur_str;
    for (unsigned hex_alphabets_idx = 0; hex_alphabets_idx < 6; hex_alphabets_idx++) {
        cur_str = hex_alphabets[hex_alphabets_idx];
        printf_x86("%s: cur_str: %s\n", __func__, cur_str);
        for (int digit_location = 0; digit_location < 6; digit_location++) {
            lcd.display_alphabet_in_location(cur_str[digit_location], digit_location);
        }
        lcd.commit();
        cpu_busy_loop_1_second();
        lcd.reset_ram_buf();
    }
    arm_inf_loop();
}

void demo_alphabets()
{
    // lcd.display_alphabet_in_location('x', 1);
    // lcd.commit();
    // arm_inf_loop();

    Lcd &lcd = Lcd::get();
    for (int i = 'a'; i < 'z'; i++) {
        lcd.display_alphabet_in_location(i, 1);
        lcd.commit();
        cpu_busy_loop_1_second();
        lcd.reset_ram_buf();
    }
    arm_inf_loop();
}

void demo_timer()
{
    Lcd &lcd = Lcd::get();
    Clocks &clocks = Clocks::get();

    unsigned single_tick_dur_ns_u = 1000000000 / 2097000;
    unsigned prescaler_1ms_per_tick = 1000000 / single_tick_dur_ns_u;
    printf_x86("Single tick: %u, prescaler_1ms_per_tick: %u\n", single_tick_dur_ns_u,
               prescaler_1ms_per_tick);

    display_int_on_lcd_for_one_second(single_tick_dur_ns_u);
    display_int_on_lcd_for_one_second(prescaler_1ms_per_tick);
    display_int_on_lcd_for_one_second(LOOPS_FOR_1_SEC_BUSY_LOOP);

    clocks.start_timer(prescaler_1ms_per_tick);
    while (1) {
        lcd.reset_ram_buf();
        lcd.write_int_to_ram_buf(clocks.read_timer());
        lcd.commit();
        // cpu_busy_loop_1_second();
        cpu_busy_loop_1_ms();
#ifdef __x86_64
        return;
#endif
    }
}

void demo_uart()
{
    UART &uart = UART::get();
    char str[7] = "000000";
    int str_idx = 0;
    uart.send_str("In UART demo, echoing back received characters\n\r");
    while (true) {
        REG data = uart.read_char();
        data &= 0xffff;
        // TODO: This only works if user sends space:
        // uart.send_str("Received: ");
        uart.send_char(data);
        // Also display on LCD
        str[str_idx] = data;
        str_idx++;
        if (str_idx > 5)
            str_idx = 0;
        display_str_on_lcd(str);
    }
}

void dump_address_over_uart(const char *msg, REG addr)
{
    UART &uart = UART::get();
    uart.send_str(msg);
    uart.send_str(" - addr: ");
    uart.send_hex((addr));
    uart.send_str(" value: ");
    uart.send_hex(*(REG *)addr);
    uart.send_newline();
}

void demo_dma()
{
    Dma &dma = Dma::get();
    UART &uart = UART::get();
    uart.send_str("DMA demo\n\r");
    // TODO: Heap not reserved yet
    // Heap
    // REG *src = (REG *)0x20001000;
    // REG *dst = (REG *)0x20002000;
    // *src = 0x12345678;
    // *dst = 0;

    // Stack
    uart.send_str("DMA test - stack - before\n\r");
    REG src;
    REG dst;
    src = 0x12345678;
    dst = 0;

    dump_address_over_uart("Source", (REG)&src);
    dump_address_over_uart("Dest  ", (REG)&dst);

    dma.reset_channel(0);
    dma.transfer_data(0, (REG)&src, (REG)&dst, 4);

    uart.send_str("DMA test - stack - after\n\r");
    dump_address_over_uart("Source", (REG)&src);
    dump_address_over_uart("Dest  ", (REG)&dst);
    display_hex_on_lcd_for_two_seconds((REG)(&dst));
    display_hex_on_lcd_for_two_seconds((REG)(dst));

    // BSS
    uart.send_str("DMA test - BSS - before\n\r");
    REG *bss_src = (REG *)(dma_demo_bss_buffer);
    REG *bss_dst = (REG *)(dma_demo_bss_buffer + 0xa00);
    *bss_src = 0x12345678;
    *bss_dst = 0;
    dump_address_over_uart("BSS Source", (REG)bss_src);
    dump_address_over_uart("BSS Dest  ", (REG)bss_dst);

    dma.reset_channel(0);
    dma.transfer_data(0, (REG)bss_src, (REG)bss_dst, 4);

    uart.send_str("DMA test - BSS - after\n\r");
    dump_address_over_uart("BSS Source", (REG)bss_src);
    dump_address_over_uart("BSS Dest  ", (REG)bss_dst);
    display_hex_on_lcd_for_two_seconds((REG)(bss_dst));
    display_hex_on_lcd_for_two_seconds((REG)(*bss_dst));
}

// NOTE: Compiler will optimize this away without -O0. Also
// with -Wall, gcc will not compile it.
#if 0
void demo_recursion(int count)
{
    display_hex_on_lcd(reinterpret_cast<REG>(&count));
    demo_recursion(++count);
}
#endif

void demo_lcd_pixels()
{
    /*
0-1 -> 0
2-3 -> 1
4-5 -> 2
6-7 -> 3
    */

    Lcd &lcd = Lcd::get();
    lcd.set_ram_buf(4, FULL_32);
    int bit_shift = 4;
    while (1) {
        lcd.reset_ram_buf();
        lcd.write_int_to_ram_buf(bit_shift);
        lcd.commit();
        cpu_busy_loop_1_second();
        cpu_busy_loop_1_second();

        lcd.reset_ram_buf();
        lcd.set_ram_buf(4, 0xf << bit_shift);
        lcd.commit();
        cpu_busy_loop_1_second();
        cpu_busy_loop_1_second();
        bit_shift += 4;
        if (bit_shift > 28)
            bit_shift = 0;
    }
}

int main()
{
    Lcd &lcd = Lcd::get();
    GPIO &gpio = GPIO::get();
    Clocks &clocks = Clocks::get();

    printf_x86("Initializing clocks and gpio\n");
    // Initialization
    clocks.init_gpio_clocks();

    gpio.set_gpio_moder_to_af_for_lcd();
    gpio.set_gpio_af_modes_for_lcd();
    gpio.set_gpio_moder_to_af_for_uart();
    gpio.set_gpio_af_modes_for_uart();

    clocks.init_clocks_for_lcd();
    clocks.init_clocks_for_uart();
    clocks.init_dma_clocks();
    lcd.init_lcd();
    lcd.reset_ram_buf();

    Dma::get();
    UART::get();

    printf_x86("Program starts\n");

    UART &uart = UART::get();
    uart.send_newline();
    uart.send_str("Main - clocks and gpio intitialized\n\r");

    demo_dma();
    // demo_uart();
    // demo_timer();
    // demo_lcd_pixels();
    // test_sram();
    // demo_alphabets();
    // demo_recursion(0);
    // demo_hex_alphabets();

    arm_inf_loop();

    return 0;
}
