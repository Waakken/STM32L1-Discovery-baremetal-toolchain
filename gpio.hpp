#include "reg_access.hpp"

class GPIO {
public:
    GPIO() :
        gpioa_reg(get_gpioa()),
        gpiob_reg(get_gpiob()),
        gpioc_reg(get_gpioc()),
        gpiod_reg(get_gpiod()),
        gpioe_reg(get_gpioe())
    {};
    void set_gpio_af_modes();
    void set_gpio_moder_to_af();
    void turn_off_led();
    void turn_on_led();
private:
    struct gpio *gpioa_reg;
    struct gpio *gpiob_reg;
    struct gpio *gpioc_reg;
    struct gpio *gpiod_reg;
    struct gpio *gpioe_reg;
};
