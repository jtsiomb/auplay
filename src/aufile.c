/*
auplay - audio player for DOS.
Copyright (C) 2019-2022  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
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
