#define GPIOA_JANI        0x40020000
#define GPIOA_ODR_JANI    0x40020014
#define RCC_AHBENR_JANI   0x4002381C

typedef long unsigned uint32_t;

void turn_on_led()
{
    volatile uint32_t *gpioa_ptr, *rcc_ahbenr_ptr, *gpio_odr_ptr;

    gpioa_ptr = (volatile uint32_t *)GPIOA_JANI;
    rcc_ahbenr_ptr = (volatile uint32_t *)RCC_AHBENR_JANI;
    gpio_odr_ptr = (volatile uint32_t *)GPIOA_ODR_JANI;
    *rcc_ahbenr_ptr = 1;
    *gpioa_ptr = 1;
    *gpio_odr_ptr = 1;
}

int main() {
    /* register unsigned long r; */
    /* asm volatile("mrs %0, control" : "=r"(r)); */
    turn_on_led();
    /* int a = 0x111; */
    /* int b = 0x222; */
    /* int c = a + b; */
    /* while(1); */
    return 0;
}
