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
