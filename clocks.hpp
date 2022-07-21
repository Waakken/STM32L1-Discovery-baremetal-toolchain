class Clocks {
public:
    void init_clocks_for_lcd();
    void delay(unsigned ms);
    void start_timer(unsigned prescaler);
    void init_gpio_clocks();
};
