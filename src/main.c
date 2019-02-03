#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include "aufile.h"
#include "audio.h"

#define DBG_PRELOAD

static int play_file(const char *fname);
static int cbfunc(void *buf, int size, void *cls);
static void print_usage(const char *argv0);

static int vol = 255;
static int quit;

int main(int argc, char **argv)
{
	int i, vol;

	if(audio_init() == -1) {
		return 1;
	}
	audio_setvolume(AUDIO_MASTER, 255);
	audio_setvolume(AUDIO_PCM, 255);

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-volume") == 0) {
				if(!argv[++i] || (vol = atoi(argv[i])) <= 0 || vol > 100) {
					fprintf(stderr, "%s must be followed by a number 1-100\n", argv[-1]);
					return 1;
				}
				audio_setvolume(AUDIO_DEFAULT, vol * 255 / 100);

			} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
				print_usage(argv[0]);
				return 0;
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return 1;
			}
		} else {
			play_file(argv[i]);
			if(quit) break;
		}
	}
	return 0;
}

static unsigned char *dbg_samples;
static int dbg_cur_offs;

static int play_file(const char *fname)
{
	struct au_file *au;
	int paused = 0, muted = 0;
	unsigned long prev;

	if(!(au = au_open(fname))) {
		return -1;
	}

	audio_set_callback(cbfunc, au);

#ifdef DBG_PRELOAD
	if(!(dbg_samples = malloc(au->size))) {
		perror("failed to preload samples into memory");
		return -1;
	}
	dbg_cur_offs = 0;
	if(au_read(au, dbg_samples, au->size) < au->size) {
		perror("failed to preload samples into memory");
		return -1;
	}
#endif

	audio_play(au->rate, au->bits, au->chan);
	while(audio_isplaying()) {
		if(kbhit()) {
			int c = getch();
			switch(c) {
			case 27:
				audio_stop();
				quit = 1;
				goto end;

			case ' ':
				paused = !paused;
				if(paused) {
					audio_pause();
					printf("\npause\n");
				} else {
					audio_resume();
					printf("resume\n");
				}
				break;

			case '=':
				vol += 32;
				if(vol > 255) vol = 255;
				audio_setvolume(AUDIO_DEFAULT, vol);
				printf("\nvolume: %d%%\n", 101 * vol / 256);
				break;

			case '-':
				vol -= 32;
				if(vol < 0) vol = 0;
				audio_setvolume(AUDIO_DEFAULT, vol);
				printf("\nvolume: %d%%\n", 101 * vol / 256);
				break;

			case 'm':
				muted = !muted;
				if(muted) {
					audio_setvolume(AUDIO_DEFAULT, 0);
				} else {
					audio_setvolume(AUDIO_DEFAULT, vol);
				}
				break;

			default:
				break;
			}
		}

		_disable();
		if(dbg_cur_offs != prev) {
			prev = dbg_cur_offs;
			_enable();

			printf("%3d%% - offs: %lu/%lu       \r", 100 * prev / au->size, prev, au->size);
			fflush(stdout);
		} else {
			_enable();
		}

	}

end:
	putchar('\n');
	au_close(au);
#ifdef DBG_PRELOAD
	free(dbg_samples);
#endif
	return 0;
}

#ifdef DBG_PRELOAD
static int cbfunc(void *buf, int size, void *cls)
{
	struct au_file *au = cls;

	if(dbg_cur_offs + size > au->size) {
		size = au->size - dbg_cur_offs;
	}
	if(size <= 0) return 0;

	memcpy(buf, dbg_samples + dbg_cur_offs, size);
	dbg_cur_offs += size;
	return size;
}
#else
static int cbfunc(void *buf, int size, void *cls)
{
	int rd;

	if((rd = au_read(cls, buf, size)) <= 0) {
		return 0;
	}
	return rd;
}
#endif

static void print_usage(const char *argv0)
{
	printf("Usage: %s [options] <file1> <file2> ... <filen>\n", argv0);
	printf("options:\n");
	printf(" -v,-volume <percent>: set audio volume (1-100)\n");
	printf(" -h,-help: print usage and exit\n");
}
