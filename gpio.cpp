#include "gpio.hpp"
#include "x86.hpp"

#define SET_GPIO_AFRL_BIT(reg, nth, val) (reg |= (val << (nth * 4)))
#define SET_GPIO_AFRH_BIT(reg, nth, val) (reg |= (val << ((nth - 8) * 4)))

void GPIO::set_gpio_af_modes_for_uart()
{
    // For UART1
    char af_7 = 7;
    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 6, af_7);
    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 7, af_7);
}

void GPIO::set_gpio_moder_to_af_for_uart()
{
    // For UART1
    REG reg;
    reg = 0b00000000'00000000'10100000'00000000;
    gpiob_reg->moder |= reg;
}

void GPIO::set_gpio_af_modes_for_lcd()
{
    // Set to AF 11
    char af_11 = 11;

    // Remove for usart2, let's call it 4 digit mode: 4 LSB digits usable
    SET_GPIO_AFRL_BIT(gpioa_reg->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(gpioa_reg->afrl, 2, af_11);
    SET_GPIO_AFRL_BIT(gpioa_reg->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(gpioa_reg->afrl, 6, af_11);
    SET_GPIO_AFRL_BIT(gpioa_reg->afrl, 7, af_11);

    SET_GPIO_AFRH_BIT(gpioa_reg->afrh, 8, af_11);
    SET_GPIO_AFRH_BIT(gpioa_reg->afrh, 9, af_11);
    SET_GPIO_AFRH_BIT(gpioa_reg->afrh, 10, af_11);
    SET_GPIO_AFRH_BIT(gpioa_reg->afrh, 15, af_11);

    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 0, af_11);
    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 1, af_11);
    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 3, af_11);
    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 4, af_11);
    SET_GPIO_AFRL_BIT(gpiob_reg->afrl, 5, af_11);

    for (char i = 8; i < 16; i++)
        SET_GPIO_AFRH_BIT(gpiob_reg->afrh, i, af_11);

    for (char i = 0; i < 8; i++)
        SET_GPIO_AFRL_BIT(gpioc_reg->afrl, i, af_11);
    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(gpioc_reg->afrh, i, af_11);

    SET_GPIO_AFRL_BIT(gpiod_reg->afrl, 2, af_11);

    for (char i = 8; i < 13; i++)
        SET_GPIO_AFRH_BIT(gpiod_reg->afrh, i, af_11);

    for (char i = 0; i < 4; i++)
        SET_GPIO_AFRL_BIT(gpioe_reg->afrl, i, af_11);
}

void GPIO::set_gpio_moder_to_af_for_lcd()
{
    REG reg;
    reg = 0b10000000'00101010'10100000'10101000;
    gpioa_reg->moder |= reg;

    reg = 0b10101010'10101010'00001010'10001010;
    gpiob_reg->moder |= reg;

    reg = 0b00000010'10101010'10101010'10101010;
    gpioc_reg->moder |= reg;

    reg = 0b10101010'10101010'00000000'00100000;
    gpiod_reg->moder |= reg;

    reg = 0b00000000'00000000'00000000'10101010;
    gpioe_reg->moder |= reg;
}

void GPIO::turn_on_led()
{
    gpioa_reg->moder |= 1;
    gpioa_reg->odr |= 1;
}

void GPIO::turn_off_led() { gpioa_reg->odr &= ~1; }
