#include <stdio.h>
#include <stdlib.h>
#include "aufile.h"
#include "inttypes.h"

struct format {
	uint16_t fmt;
	uint16_t nchan;
	uint32_t rate;
	uint16_t avgbaud;
	uint16_t block_align;
	uint16_t sample_bytes;
};

struct playback_data {
	uint32_t start, size;
	uint32_t bytes_left;
};

#define FOURCC(a, b, c, d) \
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

enum {
	ID_RIFF	= FOURCC('R', 'I', 'F', 'F'),
	ID_WAVE	= FOURCC('W', 'A', 'V', 'E'),
	ID_FMT = FOURCC('f', 'm', 't', ' '),
	ID_DATA = FOURCC('d', 'a', 't', 'a')
};

static void close_wav(struct au_file *au);
static void reset_wav(struct au_file *au);
static int read_wav(struct au_file *au, void *buf, int size);
static int read_uint32(uint32_t *res, FILE *fp);
static int read_format(struct format *fmt, int fmtsize, FILE *fp);


int au_open_wav(struct au_file *au)
{
	uint32_t id, len;
	struct format fmt;
	struct playback_data *pb;

	if(read_uint32(&id, au->fp) == -1 || id != ID_RIFF)
		return -1;
	fseek(au->fp, 4, SEEK_CUR);
	if(read_uint32(&id, au->fp) == -1 || id != ID_WAVE)
		return -1;
	if(read_uint32(&id, au->fp) == -1 || id != ID_FMT)
		return -1;
	if(read_uint32(&len, au->fp) == -1)
		return -1;
	if(read_format(&fmt, len, au->fp) == -1)
		return -1;
	if(read_uint32(&id, au->fp) == -1 || id != ID_DATA)
		return -1;
	if(read_uint32(&len, au->fp) == -1)
		return -1;

	if(!(pb = malloc(sizeof *pb))) {
		fprintf(stderr, "failed to allocate wav playback data block\n");
		return -1;
	}
	pb->start = ftell(au->fp);
	pb->size = pb->bytes_left = len;

	au->rate = fmt.rate;
	au->bits = fmt.sample_bytes * 8;
	au->chan = fmt.nchan;
	au->size = pb->size;
	au->data = pb;

	au->close = close_wav;
	au->reset = reset_wav;
	au->read = read_wav;
	return 0;
}

static void close_wav(struct au_file *au)
{
	free(au->data);
}

static void reset_wav(struct au_file *au)
{
	struct playback_data *pb = au->data;
	pb->bytes_left = pb->size;
	fseek(au->fp, pb->start, SEEK_SET);
}

static int read_wav(struct au_file *au, void *buf, int size)
{
	struct playback_data *pb = au->data;
	size_t rd;

	if(size > pb->bytes_left) {
		size = pb->bytes_left;
	}
	if(size <= 0) {
		return 0;
	}
	if((rd = fread(buf, 1, size, au->fp)) == -1) {
		pb->bytes_left = 0;
		return -1;
	}
	pb->bytes_left -= rd;
	return rd;
}

#ifdef BIGENDIAN
static void swap_uint32(uint32_t *val)
{
	uint32_t x = *val;
	*val = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
}

static void swap_uint16(uint16_t *val)
{
	uint16_t x = *val;
	*val = (x << 8) | (x >> 8);
}
#endif

static int read_uint32(uint32_t *res, FILE *fp)
{
	if(fread(res, 4, 1, fp) < 1) {
		return -1;
	}
#ifdef BIGENDIAN
	swap_uint32(res);
#endif
	return 0;
}

static int read_format(struct format *fmt, int fmtsize, FILE *fp)
{
	if(fread(fmt, 1, fmtsize, fp) < fmtsize) {
		return -1;
	}
#ifdef BIGENDIAN
	swap_uint16(&fmt->fmt);
	swap_uint16(&fmt->nchan);
	swap_uint32(&fmt->rate);
	swap_uint16(&fmt->avgbaud);
	swap_uint16(&fmt->block_align);
#endif
	return 0;
}
