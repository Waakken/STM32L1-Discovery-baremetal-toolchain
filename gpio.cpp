#include "gpio.hpp"
#include "reg_access.hpp"
#include "reg_defs.hpp"
#include "x86.hpp"

#define SET_GPIO_AFRL_BIT(reg, nth, val) (reg |= (val << (nth * 4)))
#define SET_GPIO_AFRH_BIT(reg, nth, val) (reg |= (val << ((nth - 8) * 4)))
#define SET_GPIO_MODER_BIT(reg, nth, val) (reg |= (val << (nth * 2)))

void GPIO::set_gpio_af_modes()
{
    // Set to AF 11

    char af_11 = 11;

    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 2, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 6, af_11);
    SET_GPIO_AFRL_BIT(get_gpioa()->afrl, 7, af_11);

    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 8, af_11);
    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 9, af_11);
    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 10, af_11);
    SET_GPIO_AFRH_BIT(get_gpioa()->afrh, 15, af_11);

    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 0, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 4, af_11);
    SET_GPIO_AFRL_BIT(get_gpiob()->afrl, 5, af_11);
    for (char i = 8; i < 16; i++)
        SET_GPIO_AFRH_BIT(get_gpiob()->afrh, i, af_11);

    for (char i = 0; i < 8; i++)
        SET_GPIO_AFRL_BIT(get_gpioc()->afrl, i, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(get_gpioc()->afrh, i, af_11);

    SET_GPIO_AFRL_BIT(get_gpiod()->afrl, 2, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(get_gpiod()->afrh, i, af_11);

    for (char i = 0; i < 4; i++)
        SET_GPIO_AFRL_BIT(get_gpioe()->afrl, i, af_11);
}

void GPIO::set_gpio_moder_to_af()
{
    char af = 2;

    SET_GPIO_MODER_BIT(get_gpioa()->moder, 1, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 2, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 3, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 6, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 7, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 8, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 9, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 10, af);
    SET_GPIO_MODER_BIT(get_gpioa()->moder, 15, af);

    SET_GPIO_MODER_BIT(get_gpiob()->moder, 0, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 1, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 3, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 4, af);
    SET_GPIO_MODER_BIT(get_gpiob()->moder, 5, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(get_gpiob()->moder, i, af);

    for (char i = 0; i < 13; i++)
        SET_GPIO_MODER_BIT(get_gpioc()->moder, i, af);

    SET_GPIO_MODER_BIT(get_gpiod()->moder, 2, af);
    for (char i = 8; i < 16; i++)
        SET_GPIO_MODER_BIT(get_gpiod()->moder, i, af);

    for (char i = 0; i < 5; i++)
        SET_GPIO_MODER_BIT(get_gpioe()->moder, i, af);
}

void GPIO::turn_on_led()
{
    get_gpioa()->moder |= 1;
    get_gpioa()->odr |= 1;
}

void GPIO::turn_off_led() { get_gpioa()->odr &= ~1; }
