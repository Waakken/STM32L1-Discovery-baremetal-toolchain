#define RCC_AHBENR_JANI   0x4002381C
#define RCC_APB1ENR_JANI  0x40023824

#define GPIOA_JANI        0x40020000
#define GPIOA_ODR_JANI    0x40020014

#define TIM2_BASE_JANI    0x40000000
#define TIM2_CR1_JANI     TIM2_BASE_JANI
#define TIM2_CNT_JANI     0x40000024

typedef volatile unsigned long REG;

void turn_on_led()
{
  REG *gpioa_ptr, *rcc_ahbenr_ptr, *gpio_odr_ptr;
  unsigned long val;

  rcc_ahbenr_ptr = (REG *)RCC_AHBENR_JANI;
  gpioa_ptr = (REG *)GPIOA_JANI;
  gpio_odr_ptr = (REG *)GPIOA_ODR_JANI;

  *rcc_ahbenr_ptr = *rcc_ahbenr_ptr | 1;

  *gpioa_ptr = *gpioa_ptr | 1;

  *gpio_odr_ptr = *gpio_odr_ptr | 1;

}

void turn_off_led()
{
  REG *gpioa_ptr, *rcc_ahbenr_ptr, *gpio_odr_ptr;
  unsigned long val;

  rcc_ahbenr_ptr = (REG *)RCC_AHBENR_JANI;
  gpioa_ptr = (REG *)GPIOA_JANI;
  gpio_odr_ptr = (REG *)GPIOA_ODR_JANI;

  *rcc_ahbenr_ptr = *rcc_ahbenr_ptr | 1;

  *gpioa_ptr = *gpioa_ptr | 1;

  *gpio_odr_ptr = *gpio_odr_ptr & ~1;

}

void enable_timer()
{
    REG *apb1enr_ptr, *tim2_cr1_ptr, *tim2_cnt_ptr;
    unsigned long val;

    apb1enr_ptr = (REG *)RCC_APB1ENR_JANI;
    tim2_cr1_ptr = (REG *)TIM2_CR1_JANI;
    tim2_cnt_ptr = (REG *)TIM2_CNT_JANI;

    /* enable tim2 */
    val = *apb1enr_ptr | 1;
    *apb1enr_ptr = val;
    val = *tim2_cr1_ptr | 1;
    *tim2_cr1_ptr = val;

    for (int i = 0; i < 10; i++) {
        /* Read timer value */
        val = *tim2_cnt_ptr;
    }
}

static void delay(unsigned ms)
{
    int val;
    REG *apb1enr_ptr = (REG *)RCC_APB1ENR_JANI;
    REG *tim2_base_ptr = (REG *)TIM2_CR1_JANI;
    REG *tim2_cr_ptr = (REG *)TIM2_BASE_JANI;
    REG *tim2_sr_ptr = (REG *)(TIM2_BASE_JANI + 0x10);
    REG *tim2_psc_ptr = (REG *)(TIM2_BASE_JANI + 0x28);
    REG *tim2_arr_ptr = (REG *)(TIM2_BASE_JANI + 0x2c);
    REG *tim2_cnt_ptr = (REG *)TIM2_CNT_JANI;

    /* enable tim2 */
    *apb1enr_ptr = *apb1enr_ptr | 1;

    /* unset cr1 */
    *tim2_cr_ptr = *tim2_cr_ptr & ~1;
    *tim2_sr_ptr = 0;

    /* clear counter */
    *tim2_cnt_ptr = 0;

    /* set prescaler */
    /* 15 sec */
    /* *tim2_psc_ptr = 0x2000; */

    /* 2 sec */
    *tim2_psc_ptr = 0x200;

    /* arr = ms */
    *tim2_arr_ptr = ms;

    /* enable tim2 */
    *tim2_cr_ptr = *tim2_cr_ptr | 1;

    while(!*tim2_sr_ptr);
}

int main() {
    while (1) {
        turn_on_led();
        delay(0x1000);
        turn_off_led();
        delay(0x1000);
    }
    return 0;
}
