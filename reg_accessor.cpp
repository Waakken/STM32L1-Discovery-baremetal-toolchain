#include "reg_accessor.hpp"

#ifdef __x86_64
void RegAccessor::redirect_pointers_in_x86()
{
    // printf("Redirecting pointers\n");
    // TODO: Allocate contiguous area, with size of 0x40023800 -
    // 0x40000000
    gpioa = (struct gpio *)malloc(sizeof(struct gpio));
    gpiob = (struct gpio *)malloc(sizeof(struct gpio));
    gpioc = (struct gpio *)malloc(sizeof(struct gpio));
    gpiod = (struct gpio *)malloc(sizeof(struct gpio));
    gpioe = (struct gpio *)malloc(sizeof(struct gpio));
    rcc = (struct rcc *)malloc(sizeof(struct rcc));
    tim2 = (struct tim2 *)malloc(sizeof(struct tim2));
    rtc = (struct rtc *)malloc(sizeof(struct rtc));
    lcd = (struct lcd *)malloc(sizeof(struct lcd));
    pwr = (struct pwr *)malloc(sizeof(struct pwr));
    dma1 = (struct dma_controller *)malloc(sizeof(struct dma_controller));
    dma2 = (struct dma_controller *)malloc(sizeof(struct dma_controller));

    sram = (REG *)malloc(SRAM_SIZE);
    flash = (REG *)malloc(FLASH_SIZE);
}

void RegAccessor::free_pointers_in_x86()
{
    // printf("Freeing pointers\n");
    free(gpioa);
    free(gpiob);
    free(gpioc);
    free(gpiod);
    free(gpioe);
    free(rcc);
    free(tim2);
    free(rtc);
    free(lcd);
    free(pwr);
    free(dma1);
    free(dma2);

    free((void *)sram);
    free((void *)flash);
}
#endif
