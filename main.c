#include "reg_defs.h"
#include "reg_addr.h"

struct gpio *gpioa = (struct gpio *)GPIOA;
struct gpio *gpiob = (struct gpio *)GPIOB;
struct gpio *gpioc = (struct gpio *)GPIOC;
struct gpio *gpiod = (struct gpio *)GPIOD;
struct rcc *rcc = (struct rcc *)RCC;
struct tim2 *tim2 = (struct tim2 *)TIM2;
struct rtc *rtc = (struct rtc *)RTC;
struct lcd *lcd = (struct lcd *)LCD;
struct pwr *pwr = (struct pwr *)PWR;

void init_lcd()
{
    // enable
    //lcd->cr |= 1;

    // bias 1/3
    lcd->cr |=  (1 << 6);

    // duty 1/4
    lcd->cr |= (3 << 2);

    // contrast 111
    lcd->fcr |=  (3 << 10);

    // pulse width
    lcd->fcr |= (7 << 4);

    // disable mux
    lcd->cr &= ~(1 << 7);

    // use internal voltage source
    lcd->cr &= ~1;

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
}

void init_clocks_for_lcd()
{
    // Power interface clock enable
    rcc->apb1enr |= (1 << 28);

    // pwr->cr Looks OK!
    // Disable backup write protection
    pwr->cr |=  (1 << 8);

    rtc->wpr = 0xca;
    rtc->wpr = 0x53;

    // rcc->csr Looks OK!
    // enable LSI
    rcc->csr |= 1;

    // use LCI as LCD clock
    rcc->csr |= (1 << 17);

    // enable RTC clock
    rcc->csr |= (1 << 22);
}

void init_gpio_clocks() {
    rcc->ahbenr |= 0xf;
}

void init_gpios_for_lcd()
{
    // Set to AF 11

    char af_11 = 11;
    // A - 6, 7
    gpioa->afrl |= (af_11 << 24) | (af_11 << 28);
    // A - 8, 9, 10, 15
    gpioa->afrh |= (af_11) | (af_11 << 4) | (af_11 << 8) | (af_11 << 28);

    // B - 0, 1, 4, 5
    gpiob->afrl |= (af_11) | (af_11 << 4) | (af_11 << 16) | (af_11 << 20);
    // B - 9, 12, 13, 14, 15
    gpiob->afrh |= (af_11 << 4) | (af_11 << 16) | (af_11 << 20) | (af_11 << 24) | (af_11 << 28);

    // C - 3, 4, 5, 6, 7
    gpioc->afrl |= (af_11 << 12) | (af_11 << 16) | (af_11 << 20) | (af_11 << 24) | (af_11 << 28);
    // C - 8
    gpioc->afrh |= (af_11);

    // D - 8, 9, 10, 11, 12, 13, 14, 15
    gpiod->afrh |= (af_11) | (af_11 << 4) |
        (af_11 << 8) | (af_11 << 12) | (af_11 << 16) |
        (af_11 << 20) | (af_11 << 24) | (af_11 << 28);

    /*
      TODO: Enable A, B, C, D clocks
     */
}

void turn_on_led()
{
  gpioa->moder |= 1;
  gpioa->odr |= 1;
}

void turn_off_led()
{
  gpioa->odr &= ~1;
}

static void delay(unsigned ms)
{
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

    while(!tim2->sr);
}

void blink_led(char count) {
    for (int i = 0; i < 3; i++) {
        /* TODO: Why this argument doesn't work? */
        /* for (char i = 0; i < count; i++) { */
        turn_on_led();
        delay(2000);
        turn_off_led();
        delay(2000);
    }
}

int main() {
    asm("nop");
    asm("nop");
    init_gpio_clocks();
    blink_led(3);

    init_gpios_for_lcd();
    init_clocks_for_lcd();
    asm("nop");
    asm("nop");
    asm("nop");
    init_lcd();

    /* delay(4000); */
    /* blink_led(5); */
    return 0;
}
