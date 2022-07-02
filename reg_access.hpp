#pragma once

#include "reg_addr.hpp"
struct rcc *get_rcc(void);
struct gpio *get_gpioa(void);
struct gpio *get_gpiob(void);
struct gpio *get_gpioc(void);
struct gpio *get_gpiod(void);
struct gpio *get_gpioe(void);
struct pwr *get_pwr(void);
struct tim2 *get_tim2(void);
struct rtc *get_rtc(void);
struct lcd *get_lcd(void);

void redirect_pointers_in_x86();
