#include "reg_defs.h"
#include "reg_addr.h"

struct gpio *gpioa = (struct gpio *)GPIOA;
struct gpio *gpiob = (struct gpio *)GPIOB;
struct gpio *gpioc = (struct gpio *)GPIOC;
struct gpio *gpiod = (struct gpio *)GPIOD;
struct gpio *gpioe = (struct gpio *)GPIOE;
struct rcc *rcc = (struct rcc *)RCC;
struct tim2 *tim2 = (struct tim2 *)TIM2;
struct rtc *rtc = (struct rtc *)RTC;
struct lcd *lcd = (struct lcd *)LCD;
struct pwr *pwr = (struct pwr *)PWR;

REG ram_buf[RAM_BUFS];
int ram_pixel_idx = 0;
int ram_buf_idx = 0;

#define SET_GPIO_MODER_BIT(reg, nth, val) (reg |= (val << (nth * 2)))
#define SET_GPIO_AFRL_BIT(reg, nth, val) (reg |= (val << (nth * 4)))
#define SET_GPIO_AFRH_BIT(reg, nth, val) (reg |= (val << ((nth - 8) * 4)))

// forward declarations
void blink_led(char count);

void fill_ram_buf()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        ram_buf[i] = 0xffffffff;
    }
}

void write_next_pixel()
{
    ram_buf[ram_buf_idx] |= (1 << ram_pixel_idx);
    ram_pixel_idx++;
    if (ram_pixel_idx > 31){
        ram_pixel_idx = 0;
        ram_buf_idx++;
        blink_led(3);
    }
}

void write_full_buf()
{
    ram_buf[ram_buf_idx] = 0xffffffff;
    ram_buf_idx++;
    if (ram_buf_idx > 15)
        ram_buf_idx = 0;
}

void zero_ram_buf()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        ram_buf[i] = 0;
    }
}

void copy_lcd_ram_buf()
{
    // TODO: Need to wait and see that it's safe to write

    for (int i = 0; i < RAM_BUFS; i++) {
        lcd->ram[i] = ram_buf[i];
    }

    // Update display request
    lcd->sr |= (1 << 2);
}

void show_empty_screen()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        lcd->ram[i] = 0;
    }

    // Update display request
    lcd->sr |= (1 << 2);

    /* while(1); */
}

void zero_lcd_bits()
{
    for (int i = 0; i < RAM_BUFS; i++) {
        lcd->ram[i] = 0;
    }

    // Update display request
    lcd->sr |= (1 << 2);

    /* while(1); */
}

void init_lcd()
{
    // bias 1/2
    //lcd->cr |=  (1 << 5);

    // bias 1/3
    lcd->cr |=  (1 << 6);

    // bias 1/4
    // = 0

    // duty 1/2
    //lcd->cr |= (1 << 2);

    // duty 1/3
    //lcd->cr |= (1 << 3);

    // duty 1/4
    lcd->cr |= (3 << 2);

    // duty 1/8
    //lcd->cr |= (1 << 4);

    // contrast 111
    /* lcd->fcr |=  (3 << 10); */
    lcd->fcr |=  (7 << 10);

    // pulse width
    /* lcd->fcr |= (7 << 4); */
    lcd->fcr |= (3 << 4);

    // disable mux
    lcd->cr &= ~(1 << 7);

    // use internal voltage source
    lcd->cr &= ~1;

    // enable
    lcd->cr |= 1;

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

    // rcc->csr Looks OK!
    // enable LSI
    rcc->csr |= 1;

    // use LCI as LCD clock
    rcc->csr |= (1 << 17);

    // enable RTC clock
    rcc->csr |= (1 << 22);

    // enable LCD clock
    rcc->apb1enr |= (1 << 9);
}

void init_gpio_clocks() {
    rcc->ahbenr |= 0x1f;
}

void set_gpio_moder_to_af()
{
    char af = 2;

    SET_GPIO_MODER_BIT(gpioa->moder, 1, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 2, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 3, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 6, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 7, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 8, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 9, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 10, af);
    SET_GPIO_MODER_BIT(gpioa->moder, 15, af);

    SET_GPIO_MODER_BIT(gpiob->moder, 0, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 1, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 3, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 4, af);
    SET_GPIO_MODER_BIT(gpiob->moder, 5, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(gpiob->moder, i, af);

    for (char i = 0; i < 13; i++)
        SET_GPIO_MODER_BIT(gpioc->moder, i, af);

    SET_GPIO_MODER_BIT(gpiod->moder, 2, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(gpiod->moder, i, af);

    for (char i = 0; i < 5; i++)
        SET_GPIO_MODER_BIT(gpioe->moder, i, af);
}

void set_gpio_af_modes()
{
    // Set to AF 11

    char af_11 = 11;

    SET_GPIO_AFRL_BIT(gpioa->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 2, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 6, af_11);
    SET_GPIO_AFRL_BIT(gpioa->afrl, 7, af_11);

    SET_GPIO_AFRH_BIT(gpioa->afrh, 8, af_11);
    SET_GPIO_AFRH_BIT(gpioa->afrh, 9, af_11);
    SET_GPIO_AFRH_BIT(gpioa->afrh, 10, af_11);
    SET_GPIO_AFRH_BIT(gpioa->afrh, 15, af_11);

    SET_GPIO_AFRL_BIT(gpiob->afrl, 0, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 4, af_11);
    SET_GPIO_AFRL_BIT(gpiob->afrl, 5, af_11);
    for (char i = 8; i < 16; i++)
        SET_GPIO_AFRH_BIT(gpiob->afrh, i, af_11);

    for (char i = 0; i < 8; i++)
        SET_GPIO_AFRL_BIT(gpioc->afrl, i, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(gpioc->afrh, i, af_11);

    SET_GPIO_AFRL_BIT(gpiod->afrl, 2, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(gpiod->afrh, i, af_11);

    for (char i = 0; i < 4; i++)
        SET_GPIO_AFRL_BIT(gpioe->afrl, i, af_11);
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

void delay(unsigned ms)
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

int delay_hack() {
    for (int i = 0; i < 400000; i++)
        asm("nop");
}

void blink_led(char count) {
    for (int i = 0; i < 2; i++) {
        /* TODO: Why this argument doesn't work? */
        /* for (char i = 0; i < count; i++) { */
        turn_on_led();
        /* delay(2000); */
        delay_hack();
        turn_off_led();
        /* delay(2000); */
        delay_hack();
    }
}

int main() {
    asm("nop");
    asm("nop");
    init_gpio_clocks();
    /* blink_led(3); */

    set_gpio_moder_to_af();
    set_gpio_af_modes();
    init_clocks_for_lcd();
    asm("nop");
    asm("nop");
    asm("nop");
    init_lcd();
    zero_ram_buf();

    // ready conf:
    ram_buf[0] = (0xc << 12) | (0x3 << 16);
    ram_buf[2] = (0x4 << 12) | (0x2 << 16);

    //ram_buf[0] = 0xffffffff;
    //ram_buf[2] = 0xffffffff;
    //ram_buf[3] = 0xffffffff;
    //ram_buf[4] = 0xffffffff;
    //ram_buf[5] = 0xffffffff;
    //ram_buf[6] = 0xffffffff;
    copy_lcd_ram_buf();
    while(1);

    while (1) {
        /* write_full_buf(); */
        /* fill_ram_buf(); */
        write_next_pixel();
        copy_lcd_ram_buf();
        delay(200);
        /* delay_hack(); */

        /* show_empty_screen(); */
        if (ram_buf_idx >= RAM_BUFS)
            break;
    }
    /* delay(4000); */
    /* blink_led(5); */
    while(1);

    return 0;
}
