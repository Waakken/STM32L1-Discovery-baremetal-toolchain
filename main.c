#define RCC_AHBENR_JANI   0x4002381C
#define GPIOA_JANI        0x40020000
#define GPIOA_ODR_JANI    0x40020014

void turn_on_led()
{
  volatile unsigned long *gpioa_ptr, *rcc_ahbenr_ptr, *gpio_odr_ptr;
  unsigned long val;

  rcc_ahbenr_ptr = (volatile unsigned long *)RCC_AHBENR_JANI;
  gpioa_ptr = (volatile unsigned long *)GPIOA_JANI;
  gpio_odr_ptr = (volatile unsigned long *)GPIOA_ODR_JANI;

  val = *rcc_ahbenr_ptr | 1;
  *rcc_ahbenr_ptr = val;

  val = *gpioa_ptr | 1;
  *gpioa_ptr = val;

  val = *gpio_odr_ptr | 1;
  *gpio_odr_ptr = val;
}


int main() {
    turn_on_led();
    return 0;
}
