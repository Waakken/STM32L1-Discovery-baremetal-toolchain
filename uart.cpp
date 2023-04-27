#include "uart.hpp"
#include "lcd.hpp"

void UART::init(void)
{
    // This is pretty close with: screen /dev/ttyUSB0 19200,cs8
    uart1_reg->brr = 0xd0 | (0xa >> 1);

    // over sampling 8
    uart1_reg->cr1 |= (1 << 15);

    /*
      Bit 12 M: Word length
      This bit determines the word length. It is set or cleared by software.
      0: 1 Start bit, 8 Data bits, n Stop bit
      1: 1 Start bit, 9 Data bits, n Stop bit
     */
    // uart1_reg->cr1 |= (1 << 12);

    /*
      Bits 13:12 STOP: STOP bits
      These bits are used for programming the stop bits.
      00: 1 Stop bit
      01: 0.5 Stop bit
      10: 2 Stop bits
      11: 1.5 Stop bit
     */
    // uart1_reg->cr2 &= ~(1 << 12);
    // uart1_reg->cr2 &= ~(1 << 13);

    // TX, RX enable, enable
    unsigned TX_EN = 3;
    unsigned RX_EN = 2;
    unsigned USART_EN = 13;
    REG reg = (1 << USART_EN) | (1 << TX_EN) | (1 << RX_EN);
    uart1_reg->cr1 |= reg;
}

REG UART::read_char(void)
{
    while (!(uart1_reg->sr & (1 << 5)))
        ;

    REG ret = uart1_reg->dr & 0xffff;
    return ret;
}

void UART::send_char(unsigned c)
{
    while (!(uart1_reg->sr & (1 << 7)))
        ;

    uart1_reg->dr |= c;
}

void UART::send_str(const char *str)
{
#ifdef __x86_64
    printf_x86("%s", str);
#else
    for (int i = 0; i < Lcd::my_strlen(str); i++) {
        send_char(str[i]);
    }
#endif
}

void UART::send_newline(void) { send_str("\n\r"); }

void UART::send_hex(REG hex)
{
    printf_x86("0x%8x", hex);
    for (int i = 7; i >= 0; i--) {
        unsigned c = (hex >> (i * 4)) & 0xf;
        c = Lcd::byte_to_hex_ascii(c);
#ifdef __x86_64
        // printf_x86("Char to be sent: 0x%x c: %c\n", c);
#else
        send_char(c);
#endif
    }
}

void UART::send_break() { uart1_reg->cr1 |= 1; }

void UART::dump_address_over_uart(const char *msg, REG addr)
{
    send_str(msg);
    send_str(" - addr: ");
    send_hex((addr));
    send_str(" value: ");
    send_hex(*(REG *)addr);
    send_newline();
}
