#include "dma.hpp"

void Dma::reset_channel(unsigned int ch_index)
{
    if (ch_index > DMA_CHANNEL_COUNT)
        return;

    dma_reg->ch[ch_index].ccr = 0;
}

void Dma::transfer_data(unsigned int ch_index, REG src, REG dst,
                        unsigned int bytes)
{
#ifdef __x86_64
    *(REG *)dst = *(REG *)src;
#else
    dma_reg->ch[ch_index].cpar = src;
    dma_reg->ch[ch_index].cmar = dst;
    dma_reg->ch[ch_index].cndtr = bytes;
    // enable, interrups, mem2mem, pointer increment
    dma_reg->ch[ch_index].ccr |= 0b100'0000'1100'1111;
#endif
}
