#include "clocks.hpp"
#include "x86.hpp"

void Clocks::init_clocks_for_lcd()
{
    // Power interface clock enable
    rcc_reg->apb1enr |= (1 << 28);

    // Disable backup write protection
    pwr_reg->cr |= (1 << 8);

    // enable LSI
    // use LCI as LCD clock
    // enable RTC clock
    REG reg = 0b00000000'01000010'00000000'00000001;
    rcc_reg->csr |= reg;

    // enable LCD clock
    rcc_reg->apb1enr |= (1 << 9);
}

void Clocks::init_gpio_clocks()
{
    // GPIO A-H EN
    REG reg = 0b00011111;
    rcc_reg->ahbenr |= reg;
}

void Clocks::init_dma_clocks()
{
    // DMA 1
    rcc_reg->ahbenr |= (1 << 24);

    // DMA 2
    rcc_reg->ahbenr |= (1 << 25);
}

void Clocks::start_timer(unsigned prescaler)
{
    /* enable tim2 */
    rcc_reg->apb1enr |= 1;

    /* unset cr1 */
    tim2_reg->cr1 &= ~1;
    tim2_reg->sr = 0;

    /* clear counter */
    tim2_reg->cnt = 0;

    /*
      2.097 mhz clock
      >>> 1 / 2097000 = 4.768717215069147e-07 thus 1 tick equals 476 ns

      how to achieve: 1 tick = 1ms (1M ns)?

      476 * x = 1000000
            x = 1000000 / 476
            x = 2100
    */
    tim2_reg->psc = prescaler;

    /* enable tim2 */
    tim2_reg->cr1 |= 1;
}

void Clocks::delay(unsigned ms)
{
    // TODO: This function doesn't work currently
    /* enable tim2 */
    rcc_reg->apb1enr |= 1;

    /* unset cr1 */
    tim2_reg->cr1 &= ~1;
    tim2_reg->sr = 0;

    /* clear counter */
    tim2_reg->cnt = 0;

    /* set prescaler */
    /* 15 sec */

    /* 2 sec */
    // 0x200 = 512 ->
    // >>> 1 / 2097000 = 4.768717215069147e-07 thus 1 tick equals 476 ns
    // after prescaler 512 * 476 = 243.712us
    // 2000 * psc = 486.000ms
    // makes sense?
    tim2_reg->psc = 0x200;

    /* arr = ms */
    tim2_reg->arr = ms;

    /* enable tim2 */
    tim2_reg->cr1 |= 1;

    while (!tim2_reg->sr)
        ;
}
