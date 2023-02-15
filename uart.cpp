#include "uart.hpp"

void UART::init(void)
{
    // This is pretty close with: screen /dev/ttyUSB0 19200,cs8
    uart1_reg->brr = 0x70;

    // TX, RX enable, enable
    // oversampling 16
    // 1 Start bit, 8 Data bits, n Stop bit
    // parity
    uart1_reg->cr1 |= 0b00000000'00000000'00100100'00001100;
}

char UART::read_char(void)
{
    while (!(uart1_reg->sr & (1 << 5)))
        ;

    char ret = uart1_reg->dr & 0xff;
    return ret;
}
