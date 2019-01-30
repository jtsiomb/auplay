#include <stdio.h>
#include <string.h>
#include "aufile.h"

static int play_file(const char *fname);
static void print_usage(const char *argv0);

int main(int argc, char **argv)
{
	int i;

	play_file("namarie.wav");	/* TODO remove */

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
				print_usage(argv[0]);
				return 0;
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return 1;
			}
		} else {
			play_file(argv[i]);
		}
	}
	return 0;
}

static int play_file(const char *fname)
{
	struct au_file *au;

	if(!(au = au_open(fname))) {
		return -1;
	}
	/* TODO: cont */
	au_close(au);
	return 0;
}

static void print_usage(const char *argv0)
{
	printf("Usage: %s [options] <file1> <file2> ... <filen>\n", argv0);
	printf("options:\n");
	printf(" -h,-help: print usage and exit\n");
}
