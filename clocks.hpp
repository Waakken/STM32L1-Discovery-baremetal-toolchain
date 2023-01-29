#include "reg_accessor.hpp"

class Clocks : private RegAccessor
{
public:
    Clocks()
        : rcc_reg(get_rcc())
        , tim2_reg(get_tim2())
        , pwr_reg(get_pwr()){};
    void init_clocks_for_lcd();
    void delay(unsigned ms);
    void start_timer(unsigned prescaler);
    void init_gpio_clocks();
    void init_dma_clocks();
    REG read_timer(void) { return tim2_reg->cnt; };

private:
    struct rcc *rcc_reg;
    struct tim2 *tim2_reg;
    struct pwr *pwr_reg;
};
