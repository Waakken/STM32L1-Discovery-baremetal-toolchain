#pragma once

typedef volatile unsigned long REG;

struct gpio {
    REG moder;
    REG otyper;
    REG ospeedr;
    REG pupdr;
    REG idr;
    REG odr;
    REG bsrr;
    REG lckr;
    REG afrl;
    REG afrh;
    REG brr;
};

struct rcc {
    REG cr;
    REG icscr;
    REG cfgr;
    REG cir;
    REG ahbrstr;
    REG apb2rstr;
    REG apb1rstr;
    REG ahbenr;
    REG apb2enr;
    REG apb1enr;
    REG ahblpenr;
    REG apb2lpenr;
    REG apb1lp;
    REG csr;
};

struct tim2 {
    REG cr1;
    REG cr2;
    REG smcr;
    REG dier;
    REG sr;
    REG egr;
    REG ccmr1;
    REG ccmr2;
    REG ccer;
    REG cnt;
    REG psc;
    REG arr;
    REG ccr1;
    REG ccr2;
    REG ccr3;
    REG ccr4;
    REG dcr;
    REG dmar;
};

struct rtc {
    REG tr;
    REG dr;
    REG cr;
    REG isr;
    REG prer;
    REG wutr;
    REG calibr;
    REG alrmar;
    REG alrmbr;
    REG wpr;
    REG ssr;
    REG shiftr;
    REG tstr;
    REG tsdr;
    REG tsssr;
    REG calr;
    REG tafcr;
    REG alrmassr;
    REG alrmbssr;
};

#define RAM_BUFS (4 * 2)
struct lcd {
    REG cr;
    REG fcr;
    REG sr;
    REG clr;
    REG reserved;
    REG ram[RAM_BUFS];
};

struct pwr {
    REG cr;
    REG csr;
};

struct dma_general {
    REG isr;
    REG ifcr;
};

struct dma_ch {
    REG ccr;
    REG cndtr;
    REG cpar;
    REG cmar;
};

constexpr unsigned int DMA_CHANNEL_COUNT{7};

struct dma_controller {
    struct dma_general gen;
    struct dma_ch ch[DMA_CHANNEL_COUNT];
};

struct uart {
    REG sr;
    REG dr;
    REG brr;
    REG cr1;
    REG cr2;
    REG cr3;
    REG gtpr;
};

#define NVIC_PADDING 0x74

struct nvic {
    REG iser0;
    REG iser1;
    REG iser2;
    // TODO: Check is valid first
    char padding0[NVIC_PADDING];

    REG icer0;
    REG icer1;
    REG icer2;
    // TODO: Check is valid first
    char padding1[NVIC_PADDING];

    REG ispr0;
    REG ispr1;
    REG ispr2;
    // TODO: Check is valid first
    char padding2[NVIC_PADDING];

    REG icpr0;
    REG icpr1;
    REG icpr2;
    // TODO: Check is valid first
    char padding3[NVIC_PADDING];

    REG iabr0;
    REG iabr1;
    REG iabr2;
    // TODO: Check is valid first
    char padding4[NVIC_PADDING];
};
