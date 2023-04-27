#include "uart.hpp"

extern "C" {
void genericInterruptHandler(void)
{
    UART &uart = UART::get();
    REG interruptNumber = 0;
#ifndef __x86_64
    asm("mrs %0, ipsr" : "=r"(interruptNumber));
#endif

    uart.send_str("Entered interrupt handler. Interrupt number: 0x");
    uart.send_hex(interruptNumber);
    uart.send_newline();

    while (1)
        ;
}
}
