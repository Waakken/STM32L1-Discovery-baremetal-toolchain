#include "reg_defs.h"
#include "reg_addr.h"

struct gpio *gpioa = (struct gpio *)GPIOA;
struct gpio *gpiob = (struct gpio *)GPIOB;
struct gpio *gpioc = (struct gpio *)GPIOC;
struct gpio *gpiod = (struct gpio *)GPIOD;
struct rcc *rcc = (struct rcc *)RCC;
struct tim2 *tim2 = (struct tim2 *)TIM2;
struct rtc *rtc = (struct rtc *)RTC;

void init_lcd_clock()
{
    rtc->wpr = 0xca;
    rtc->wpr = 0x53;
}


void init_gpios_for_lcd()
{
    // Set to AF 11

    char af_11 = 11;
    // A - 6, 7
    gpioa->afrl = gpioa->afrl | (af_11 << 24) | (af_11 << 28);
    // A - 8, 9, 10, 15
    gpioa->afrh = gpioa->afrh | (af_11) | (af_11 << 4) | (af_11 << 8) | (af_11 << 28);

    // B - 0, 1, 4, 5
    gpiob->afrl = gpiob->afrl | (af_11) | (af_11 << 4) | (af_11 << 16) | (af_11 << 20);
    // B - 9, 12, 13, 14, 15
    gpiob->afrh = gpiob->afrh | (af_11 << 4) | (af_11 << 16) | (af_11 << 20) | (af_11 << 24) | (af_11 << 28);

    // C - 3, 4, 5, 6, 7
    gpioc->afrl = gpioc->afrl | (af_11 << 12) | (af_11 << 16) | (af_11 << 20) | (af_11 << 24) | (af_11 << 28);
    // C - 8
    gpioc->afrh = gpioc->afrh | (af_11);

    // D - 8, 9, 10, 11, 12, 13, 14, 15
    gpiod->afrh = gpiod->afrh | (af_11) | (af_11 << 4) |
        (af_11 << 8) | (af_11 << 12) | (af_11 << 16) |
        (af_11 << 20) | (af_11 << 24) | (af_11 << 28);

    /*
      TODO: Enable A, B, C, D clocks
     */
}

void turn_on_led()
{
  rcc->ahbenr = rcc->ahbenr | 1;

  gpioa->moder = gpioa->moder | 1;
  gpioa->odr = gpioa->odr | 1;
}

void turn_off_led()
{
  rcc->ahbenr = rcc->ahbenr | 1;
  gpioa->odr = gpioa->odr & ~1;
}

static void delay(unsigned ms)
{
    /* enable tim2 */
    rcc->apb1enr = rcc->apb1enr | 1;

    /* unset cr1 */
    tim2->cr1 = tim2->cr1 & ~1;
    tim2->sr = 0;

    /* clear counter */
    tim2->cnt = 0;

    /* set prescaler */
    /* 15 sec */

    /* 2 sec */
    tim2->psc = 0x200;

    /* arr = ms */
    tim2->arr = ms;

    /* enable tim2 */
    tim2->cr1 = tim2->cr1 | 1;

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
    blink_led(3);
    init_gpios_for_lcd();

    /* delay(4000); */
    /* blink_led(5); */
    return 0;
}
