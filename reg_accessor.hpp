#pragma once

#ifdef __x86_64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "reg_defs.hpp"

#define TIM2 0x40000000
#define LCD 0x40002400
#define RTC 0x40002800
#define PWR 0x40007000
#define GPIOA 0x40020000
#define GPIOB 0x40020400
#define GPIOC 0x40020800
#define GPIOD 0x40020C00
#define GPIOE 0x40021000
#define RCC 0x40023800
#define DMA1 0x40026000
#define DMA2 0x40026400
#define UART1 0x40013800
#define UART2 0x40004400
#define UART3 0x40004800
#define UART4 0x40004C00
#define UART5 0x40005000

#define SRAM 0x20000000
#define SRAM_SIZE 0x7FFF
#define FLASH 0x08000000
#define FLASH_SIZE 0x3FFFF

class RegAccessor
{
public:
    RegAccessor()
        : gpioa((struct gpio *)GPIOA)
        , gpiob((struct gpio *)GPIOB)
        , gpioc((struct gpio *)GPIOC)
        , gpiod((struct gpio *)GPIOD)
        , gpioe((struct gpio *)GPIOE)
        , rcc((struct rcc *)RCC)
        , tim2((struct tim2 *)TIM2)
        , rtc((struct rtc *)RTC)
        , lcd((struct lcd *)LCD)
        , pwr((struct pwr *)PWR)
        , dma1((struct dma_controller *)DMA1)
        , dma2((struct dma_controller *)DMA2)
        , uart1((struct uart *)UART1)
        , uart2((struct uart *)UART2)
        , uart3((struct uart *)UART3)
        , uart4((struct uart *)UART4)
        , uart5((struct uart *)UART5)
        , sram((REG *)SRAM)
        , flash((REG *)FLASH)
    {
#ifdef __x86_64
        redirect_pointers_in_x86();
#endif
    };
    ~RegAccessor()
    {
#ifdef __x86_64
        free_pointers_in_x86();
#endif
    }
    RegAccessor(const RegAccessor &) = delete;
    RegAccessor &operator=(const RegAccessor &) = delete;
    RegAccessor(RegAccessor &&) = delete;
    RegAccessor &&operator=(RegAccessor &&) = delete;

protected:
    struct rcc *get_rcc(void) { return rcc; }
    struct gpio *get_gpioa(void) { return gpioa; }
    struct gpio *get_gpiob(void) { return gpiob; }
    struct gpio *get_gpioc(void) { return gpioc; }
    struct gpio *get_gpiod(void) { return gpiod; }
    struct gpio *get_gpioe(void) { return gpioe; }
    struct pwr *get_pwr(void) { return pwr; }
    struct tim2 *get_tim2(void) { return tim2; }
    struct rtc *get_rtc(void) { return rtc; }
    struct lcd *get_lcd(void) { return lcd; }
    struct dma_controller *get_dma1(void) { return dma1; }
    struct dma_controller *get_dma2(void) { return dma2; }
    struct uart *get_uart1(void) { return uart1; }
    struct uart *get_uart2(void) { return uart2; }
    struct uart *get_uart3(void) { return uart3; }
    struct uart *get_uart4(void) { return uart4; }
    struct uart *get_uart5(void) { return uart5; }

    REG *get_sram(void) { return sram; }
    REG *get_flash(void) { return flash; }
    REG get_sram_size(void) { return SRAM_SIZE; }
    REG get_flash_size(void) { return FLASH_SIZE; }

private:
    void redirect_pointers_in_x86();
    void free_pointers_in_x86();

    struct gpio *gpioa;
    struct gpio *gpiob;
    struct gpio *gpioc;
    struct gpio *gpiod;
    struct gpio *gpioe;
    struct rcc *rcc;
    struct tim2 *tim2;
    struct rtc *rtc;
    struct lcd *lcd;
    struct pwr *pwr;
    struct dma_controller *dma1;
    struct dma_controller *dma2;
    struct uart *uart1;
    struct uart *uart2;
    struct uart *uart3;
    struct uart *uart4;
    struct uart *uart5;

    REG *sram;
    REG *flash;
};
