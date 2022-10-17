#pragma once

#include "reg_defs.hpp"
#include "reg_access.hpp"

class Dma {
public:
    Dma() : dma_reg(get_dma1()){};
    void reset_channel(unsigned int ch_index);
    void transfer_data(unsigned int ch_index, REG src, REG dst, unsigned int bytes);

private:
    struct dma_controller *dma_reg;
};
