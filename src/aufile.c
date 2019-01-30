#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "aufile.h"

int au_open_wav(struct au_file *au);

struct au_file *au_open(const char *fname)
{
	FILE *fp;
	struct au_file *au;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "au_open: failed to open file: %s: %s\n", fname, strerror(errno));
		return 0;
	}

	if(!(au = malloc(sizeof *au))) {
		fclose(fp);
		fprintf(stderr, "au_open: failed to allocate file structure: %s\n", strerror(errno));
		return 0;
	}
	au->fp = fp;

	if(au_open_wav(au) != -1) {
		return au;
	}

	fprintf(stderr, "au_open: invalid file: %s\n", fname);
	fclose(fp);
	free(au);
	return 0;
}

void au_close(struct au_file *au)
{
	au->close(au);
	fclose(au->fp);
	free(au);
}

void au_reset(struct au_file *au)
{
	au->reset(au);
}

int au_read(struct au_file *au, void *buf, int size)
{
	return au->read(au, buf, size);
}
