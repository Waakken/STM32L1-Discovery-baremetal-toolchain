#ifdef __x86_64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "reg_defs.h"

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

void redirect_pointers_in_x86()
{
#ifdef __x86_64
    printf("Redirecting pointers\n");
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
#else
    return;
#endif
}
