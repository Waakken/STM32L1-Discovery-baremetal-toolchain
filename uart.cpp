#include "uart.hpp"

void UART::init(void)
{
    // This is pretty close with: screen /dev/ttyUSB0 19200,cs8
    uart1_reg->brr = 0x70;

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

void UART::write_char(char c)
{
    // TODO: Check that HW is ready
    uart1_reg->dr |= c;
}
