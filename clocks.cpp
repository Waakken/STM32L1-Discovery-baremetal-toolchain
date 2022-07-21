#include "clocks.hpp"
#include "reg_access.hpp"
#include "reg_defs.hpp"
#include "x86.hpp"

void Clocks::init_clocks_for_lcd()
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

void Clocks::init_gpio_clocks()
{
    // GPIO A-H EN
    get_rcc()->ahbenr |= 0x1f;
}

void Clocks::start_timer(unsigned prescaler)
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

void Clocks::delay(unsigned ms)
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
