#include <conio.h>
#include "dma.h"

#define MODE_CHAN(x)	((x) & 3)
#define MODE_WRITE		0x04
#define MODE_READ		0x08
#define MODE_AUTO		0x10
#define MODE_DECR		0x20
#define MODE_SINGLE		0x40
#define MODE_BLOCK		0x80
#define MODE_CASCADE	0xc0

#define MASK_CHAN(x)	((x) & 3)
#define MASK_DISABLE	0x04

#define RMASK_CHAN(x)	(1 << ((x) & 3))

#define IS_16BIT(c)	((c) >= 4)

static void dma_io(int chan, uint32_t phyaddr, int size, unsigned int flags, unsigned int dir);
static __inline void mask(int chan);
static __inline void unmask(int chan);

/* DMA register I/O ports for all channels */
static const int addr_port[] = { 0x00, 0x02, 0x04, 0x06, 0xc0, 0xc4, 0xc8, 0xcc };
static const int count_port[] = { 0x01, 0x03, 0x05, 0x07, 0xc2, 0xc6, 0xca, 0xce };
static const int page_port[] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };
static const int mask_port[] = { 0x0a, 0x0a, 0x0a, 0x0a, 0xd4, 0xd4, 0xd4, 0xd4 };
static const int mode_port[] = { 0x0b, 0x0b, 0x0b, 0x0b, 0xd6, 0xd6, 0xd6, 0xd6 };
static const int clrff_port[] = { 0x0c, 0x0c, 0x0c, 0x0c, 0xd8, 0xd8, 0xd8, 0xd8 };

void dma_out(int chan, uint32_t phyaddr, int size, unsigned int flags)
{
	dma_io(chan, phyaddr, size, flags, MODE_READ);
}

void dma_in(int chan, uint32_t phyaddr, int size, unsigned int flags)
{
	dma_io(chan, phyaddr, size, flags, MODE_WRITE);
}

static void dma_io(int chan, uint32_t phyaddr, int size, unsigned int flags, unsigned int dir)
{
	unsigned int mode;
	unsigned char page;

	mask(chan);
	outp(clrff_port[chan], 0);

	/* first 2 bits of flags correspond to the mode bits 6,7 */
	mode = ((flags & 3) << 6) | dir | MODE_CHAN(chan);
	if(flags & DMA_DECR) mode |= MODE_DECR;
	if(flags & DMA_AUTO) mode |= MODE_AUTO;
	outp(mode_port[chan], mode);

	page = (phyaddr >> 16) & 0xff;

	if(IS_16BIT(chan)) {
		phyaddr >>= 1;
		size >>= 1;
	}

	outp(addr_port[chan], phyaddr & 0xff);
	outp(addr_port[chan], (phyaddr >> 8) & 0xff);
	outp(page_port[chan], page);

	size--;
	outp(count_port[chan], size & 0xff);
	outp(count_port[chan], (size >> 8) & 0xff);

	unmask(chan);
}

static __inline void mask(int chan)
{
	outp(mask_port[chan], MASK_CHAN(chan) | MASK_DISABLE);
}

static __inline void unmask(int chan)
{
	outp(mask_port[chan], MASK_CHAN(chan));
}
