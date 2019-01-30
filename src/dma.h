#ifndef DMA_H_
#define DMA_H_

#include <inttypes.h>

enum {
	DMA_SINGLE	= 0x01,
	DMA_BLOCK	= 0x02,
	DMA_CASCADE = DMA_SINGLE | DMA_BLOCK,
	DMA_DECR	= 0x08,
	DMA_AUTO	= 0x10
};

void dma_out(int chan, uint32_t phyaddr, int size, unsigned int flags);
void dma_in(int chan, uint32_t phyaddr, int size, unsigned int flags);

#endif	/* DMA_H_ */
