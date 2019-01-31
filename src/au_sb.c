#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dos.h>
#include <conio.h>
#include "audio.h"
#include "au_sb.h"
#include "dpmi.h"
#include "dma.h"
#include "intr.h"

#define BUFSIZE			16384

#define REG_MIXPORT		(base_port + 0x4)
#define REG_MIXDATA		(base_port + 0x5)
#define REG_RESET		(base_port + 0x6)
#define REG_RDATA		(base_port + 0xa)
#define REG_WDATA		(base_port + 0xc)
#define REG_WSTAT		(base_port + 0xc)
#define REG_RSTAT		(base_port + 0xe)
#define REG_INTACK		(base_port + 0xe)
#define REG_INT16ACK	(base_port + 0xf)

#define WSTAT_BUSY		0x80
#define RSTAT_RDY		0x80

#define DSP_RATE			0x40
#define DSP4_OUT_RATE		0x41
#define DSP4_IN_RATE		0x42
#define DSP_GET_VER			0xe1

/* start DMA playback/recording. combine with fifo/auto/input flags */
#define DSP4_START_DMA8		0xc0
#define DSP4_START_DMA16	0xb0
#define DSP4_FIFO			0x02
#define DSP4_AUTO			0x04
#define DSP4_INPUT			0x08

/* transfer mode commands */
#define DSP4_MODE_SIGNED	0x10
#define DSP4_MODE_STEREO	0x20

/* immediately pause/continue */
#define DSP_PAUSE_DMA8		0xd0
#define DSP_ENABLE_OUTPUT	0xd1
#define DSP_DISABLE_OUTPUT	0xd3
#define DSP_CONT_DMA8		0xd4
#define DSP_PAUSE_DMA16		0xd5
#define DSP_CONT_DMA16		0xd6

/* end the playback at the end of the current buffer */
#define DSP_END_DMA16		0xd9
#define DSP_END_DMA8		0xda

/* mixer registers */
#define MIX_MASTER			0x02
#define MIX_VOICE			0x0a
#define MIX_SBPRO_VOICE		0x04
#define MIX_SBPRO_MASTER	0x22
#define MIX_SB16_MASTER_L	0x30
#define MIX_SB16_MASTER_R	0x31
#define MIX_SB16_VOICE_L	0x32
#define MIX_SB16_VOICE_R	0x33
#define MIX_SB16_IRQ_SEL	0x80
#define MIX_SB16_DMA_SEL	0x81

#define VER_MAJOR(x)	((x) >> 8)
#define VER_MINOR(x)	((x) & 0xff)

/* delay for about 1us */
#define iodelay()	outp(0x80, 0)

static void INTERRUPT intr_handler();
static void start_dsp4(int bits, unsigned int mode, int num_samples);
static void start_dsp(int nchan, int num_samples);
static void write_dsp(unsigned char val);
static unsigned char read_dsp(void);
static void write_mix(unsigned char val, int reg);
static unsigned char read_mix(int reg);
static int get_dsp_version(void);
static int dsp4_detect_irq(void);
static int dsp4_detect_dma(void);
static void print_dsp_info(void);
static const char *sbname(int ver);

static int base_port;
static int irq, dma_chan, dma16_chan, dsp_ver;
static int dsp4;
static int xfer_mode;

static void *buffer[2];
static int wrbuf;

static int isplaying;
static int cur_bits, cur_mode;

static void (INTERRUPT *prev_intr_handler)();


int sb_detect(void)
{
	int i;
	char *env;

	if((env = getenv("BLASTER"))) {
		dma16_chan = -1;
		if(sscanf(env, "A%x I%d D%d H%d", &base_port, &irq, &dma_chan, &dma16_chan) >= 3) {
			if(sb_reset_dsp() == 0) {
				dsp_ver = get_dsp_version();
				dsp4 = VER_MAJOR(dsp_ver) >= 4 ? 1 : 0;
				print_dsp_info();
				return 1;
			}
		} else {
			printf("sb_detect: malformed BLASTER environment variable. Fallback to probing.\n");
		}
	}

	for(i=0; i<6; i++) {
		base_port = 0x200 + ((i + 1) << 4);
		if(sb_reset_dsp() == 0) {
			dsp_ver = get_dsp_version();
			dsp4 = VER_MAJOR(dsp_ver) >= 4 ? 1 : 0;

			if(dsp4) {
				if(dsp4_detect_irq() == -1) {
					printf("sb_detect: failed to configure IRQ\n");
					return 0;
				}
				if(dsp4_detect_dma() == -1) {
					printf("sb_detect: failed to configure DMA\n");
					return 0;
				}

			} else {
				/* XXX for old sound blasters, hard-code to IRQ 5 DMA 1 for now */
				irq = 5;
				dma_chan = 1;
				dma16_chan = -1;

				printf("sb_detect: old sound blaster dsp. assuming: irq 5, dma 1\n");
			}
			print_dsp_info();

			return 1;
		}
	}

	return 0;
}

int sb_reset_dsp(void)
{
	int i;

	outp(REG_RESET, 1);
	for(i=0; i<3; i++) iodelay();
	outp(REG_RESET, 0);

	for(i=0; i<128; i++) {
		if(inp(REG_RSTAT) & RSTAT_RDY) {
			if(inp(REG_RDATA) == 0xaa) {
				return 0;
			}
		}
	}

	return -1;
}

void sb_set_output_rate(int rate)
{
	if(dsp4) {
		write_dsp(DSP4_OUT_RATE);
		write_dsp(rate >> 8);
		write_dsp(rate & 0xff);
	} else {
		int tcon = 256 - 1000000 / rate;
		write_dsp(DSP_RATE);
		write_dsp(tcon);
	}
}

void *sb_buffer(int *size)
{
	*size = BUFSIZE / 2;
	return buffer[wrbuf];
}

void sb_start(int rate, int bits, int nchan)
{
	uint16_t seg, pmsel;
	uint32_t addr;
	int size;

	if(!buffer[0]) {
		/* allocate a 64k-aligned buffer in low memory */
		if(!(seg = dpmi_alloc(BUFSIZE * 2 / 16, &pmsel))) {
			fprintf(stderr, "sb_start: failed to allocate DMA buffer\n");
			return;
		}

		printf("DBG allocated seg: %x, addr: %lx\n", (unsigned int)seg,
				(unsigned long)seg << 4);

		addr = ((uint32_t)(seg << 4) + 0xffff) & 0xffff0000;
		printf("DBG aligned: %lx\n", (unsigned long)addr);
		buffer[0] = (void*)addr;
		buffer[1] = (void*)(addr + BUFSIZE / 2);
		wrbuf = 0;
	}

	if(!(size = audio_callback(buffer[wrbuf], BUFSIZE / 2))) {
		return;
	}
	addr = (uint32_t)buffer[wrbuf];
	wrbuf ^= 1;

	_disable();
	if(!prev_intr_handler) {
		prev_intr_handler = _dos_getvect(IRQ_TO_INTR(irq));
	}
	printf("setting interrupt vector: %d\n", IRQ_TO_INTR(irq));
	_dos_setvect(IRQ_TO_INTR(irq), intr_handler);
	_enable();

	unmask_irq(irq);

	cur_bits = bits;
	cur_mode = bits == 8 ? 0 : DSP4_MODE_SIGNED;
	if(nchan > 1) {
		cur_mode |= DSP4_MODE_STEREO;
	}

	write_dsp(DSP_ENABLE_OUTPUT);
	dma_out(bits == 8 ? dma_chan : dma16_chan, addr, size, DMA_SINGLE);
	sb_set_output_rate(rate);
	start_dsp4(cur_bits, cur_mode, bits == 8 ? size : size / 2);
	isplaying = 1;
}

static void start_dsp4(int bits, unsigned int mode, int num_samples)
{
	unsigned char cmd = bits == 8 ? DSP4_START_DMA8 : DSP4_START_DMA16;
	assert(bits == 8 || bits == 16);

	/*cmd |= DSP4_AUTO | DSP4_FIFO;*/

	/* program the DSP to start the DMA transfer */
	write_dsp(cmd);
	write_dsp(mode);
	num_samples--;
	write_dsp(num_samples & 0xff);
	write_dsp((num_samples >> 8) & 0xff);
}

static void start_dsp(int nchan, int num_samples)
{
	/* program the DSP to start the DMA transfer */
	/* TODO */
}

void sb_pause(void)
{
	write_dsp(cur_bits == 8 ? DSP_PAUSE_DMA8 : DSP_PAUSE_DMA16);
}

void sb_continue(void)
{
	write_dsp(cur_bits == 8 ? DSP_CONT_DMA8 : DSP_CONT_DMA16);
}

void sb_stop(void)
{
	write_dsp(DSP_DISABLE_OUTPUT);

	mask_irq(irq);
	/* TODO: don't _enable, restore state */
	_disable();
	_dos_setvect(IRQ_TO_INTR(irq), prev_intr_handler);
	_enable();

	/* no need to stop anything if we're in single-cycle mode */
	if(cur_mode & DSP4_AUTO) {
		write_dsp(cur_bits == 8 ? DSP_END_DMA8 : DSP_END_DMA16);
	}
	isplaying = 0;
}

int sb_isplaying(void)
{
	return isplaying;
}

void sb_volume(int vol)
{
	/* TODO */
}

static void INTERRUPT intr_handler()
{
	int size;
	void *bptr = buffer[wrbuf];
	uint32_t addr = (uint32_t)bptr;

	wrbuf ^= 1;

	/* ask for more data */
	if(!(size = audio_callback(bptr, BUFSIZE / 2))) {
		sb_stop();
	} else {
		if(cur_bits == 8) {
			dma_out(dma_chan, addr, size, DMA_SINGLE);
			start_dsp4(cur_bits, cur_mode, size);
		} else {
			dma_out(dma16_chan, addr, size, DMA_SINGLE);
			start_dsp4(cur_bits, cur_mode, size / 2);
		}
	}

	/* acknowledge the interrupt */
	if(cur_bits == 8) {
		inp(REG_INTACK);
	} else {
		inp(REG_INT16ACK);
	}

	if(irq > 7) {
		outp(PIC2_CMD, OCW2_EOI);
	}
	outp(PIC1_CMD, OCW2_EOI);
}

static void write_dsp(unsigned char val)
{
	while(inp(REG_WSTAT) & WSTAT_BUSY);
	outp(REG_WDATA, val);
}

static unsigned char read_dsp(void)
{
	while((inp(REG_RSTAT) & RSTAT_RDY) == 0);
	return inp(REG_RDATA);
}

static void write_mix(unsigned char val, int reg)
{
	outp(REG_MIXPORT, reg);
	outp(REG_MIXDATA, val);
}

static unsigned char read_mix(int reg)
{
	outp(REG_MIXPORT, reg);
	return inp(REG_MIXDATA);
}

static int get_dsp_version(void)
{
	int major, minor;

	write_dsp(DSP_GET_VER);
	major = read_dsp();
	minor = read_dsp();

	return (major << 8) | minor;
}

static int dsp4_detect_irq(void)
{
	int i, irqsel;
	static int irqtab[] = {2, 5, 7, 10};

	irq = 0;
	irqsel = read_mix(MIX_SB16_IRQ_SEL);
	for(i=0; i<4; i++) {
		if(irqsel & (1 << i)) {
			irq = irqtab[i];
			break;
		}
	}
	if(!irq) {
		/* try to force IRQ 5 */
		write_mix(2, MIX_SB16_IRQ_SEL);	/* bit1 selects irq 5 */

		/* re-read to verify */
		irqsel = read_mix(MIX_SB16_IRQ_SEL);
		if(irqsel != 2) {
			return -1;
		}
		irq = 5;
	}

	return irq;
}

static int dsp4_detect_dma(void)
{
	int i, dmasel, tmp;
	static int dmatab[] = {0, 1, -1, 3, -1, 5, 6, 7};

	dma_chan = -1;
	dma16_chan = -1;
	dmasel = read_mix(MIX_SB16_DMA_SEL);
	for(i=0; i<4; i++) {
		if(dmasel & (1 << i)) {
			dma_chan = dmatab[i];
			break;
		}
	}
	for(i=5; i<8; i++) {
		if(dmasel & (1 << i)) {
			dma16_chan = dmatab[i];
			break;
		}
	}
	if(dma_chan == -1) {
		/* try to force DMA 1 */
		dmasel |= 2;
	}
	if(dma16_chan == -1) {
		/* try to force 16bit DMA 5 */
		dmasel |= 0x20;
	}

	if(dma_chan == -1 || dma16_chan == -1) {
		write_mix(dmasel, MIX_SB16_DMA_SEL);

		/* re-read to verify */
		tmp = read_mix(MIX_SB16_DMA_SEL);
		if(tmp != dmasel) {
			return -1;
		}
		dma_chan = 1;
		dma16_chan = 5;
	}

	return dma_chan;
}

static void print_dsp_info(void)
{
	int master[2], voice[2];
	int val;

	printf("sb_detect: %s (DSP v%d.%02d) port %xh, irq %d", sbname(dsp_ver),
			VER_MAJOR(dsp_ver), VER_MINOR(dsp_ver), base_port, irq);
	if(VER_MAJOR(dsp_ver) >= 4) {
		printf(" dma %d/%d\n", dma_chan, dma16_chan);
	} else {
		printf(" dma %d\n", dma_chan);
	}

	if(VER_MAJOR(dsp_ver) >= 4) {
		master[0] = 100 * (int)(read_mix(MIX_SB16_MASTER_L) >> 3) / 31;
		master[1] = 100 * (int)(read_mix(MIX_SB16_MASTER_R) >> 3) / 31;
		voice[0] = 100 * (int)(read_mix(MIX_SB16_VOICE_L) >> 3) / 31;
		voice[1] = 100 * (int)(read_mix(MIX_SB16_VOICE_R) >> 3) / 31;
	} else if(VER_MAJOR(dsp_ver) >= 3) {
		val = read_mix(MIX_SBPRO_MASTER);
		master[0] = 100 * (val >> 5) / 7;
		master[1] = 100 * ((val >> 1) & 7) / 7;
		val = read_mix(MIX_SBPRO_VOICE);
		voice[0] = 100 * (val >> 5) / 7;
		voice[1] = 100 * ((val >> 1) & 7) / 7;
	} else {
		val = read_mix(MIX_MASTER);
		master[0] = master[1] = 100 * ((val >> 1) & 7) / 7;
		val = read_mix(MIX_VOICE);
		voice[0] = voice[1] = 100 * ((val >> 1) & 3) / 3;
	}
	printf("  mixer: master(%d|%d) voice(%d|%d)\n", master[0], master[1],
			voice[0], voice[1]);
}

#define V(maj, min)	(((maj) << 8) | (min))

static const char *sbname(int ver)
{
	int major = VER_MAJOR(ver);
	int minor = VER_MINOR(ver);

	switch(major) {
	case 1:
		if(minor == 5) {
			return "Sound Blaster 1.5";
		}
		return "Sound Blaster 1.0";

	case 2:
		if(minor == 1 || minor == 2) {
			return "Sound Blaster 2.0";
		}
		break;

	case 3:
		switch(minor) {
		case 0:
			return "Sound Blaster Pro";
		case 1:
		case 2:
			return "Sound Blaster Pro 2";
		case 5:
			return "Gallant SC-6000";
		default:
			break;
		}
		break;

	case 4:
		switch(minor) {
		case 4:
		case 5:
			return "Sound Blaster 16";
		case 11:
			return "Sound Blaster 16 SCSI-2";
		case 12:
			return "Sound Blaster AWE 32";
		case 13:
			return "Sound Blaster ViBRA16C";
		case 16:
			return "Sound Blaster AWE 64";
		default:
			break;
		}
		break;
	}

	return "Unknown Sound Blaster";
}
