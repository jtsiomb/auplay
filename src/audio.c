#include <stdio.h>
#include "audio.h"
#include "au_sb.h"

struct audrv {
	void *(*get_buffer)(int *size);
	void (*start)(int rate, int bits, int nchan);
	void (*pause)(void);
	void (*cont)(void);
	void (*stop)(void);
	void (*volume)(int vol);
	int (*isplaying)(void);
};

static struct audrv drv;

static audio_callback_func cbfunc;
static void *cbcls;

int audio_init(void)
{
	if(sb_detect()) {
		drv.get_buffer = sb_buffer;
		drv.start = sb_start;
		drv.pause = sb_pause;
		drv.cont = sb_continue;
		drv.stop = sb_stop;
		drv.volume = sb_volume;
		drv.isplaying = sb_isplaying;
		return 0;
	}

	fprintf(stderr, "No supported audio device detected\n");
	return -1;
}

void audio_set_callback(audio_callback_func func, void *cls)
{
	cbfunc = func;
	cbcls = cls;
}

int audio_callback(void *buf, int sz)
{
	if(!cbfunc) {
		return 0;
	}
	return cbfunc(buf, sz, cbcls);
}

void audio_play(int rate, int bits, int nchan)
{
	printf("play %d samples/s, %d bits, %s\n", rate, bits, nchan == 1 ? "mono" : "stereo");
	drv.start(rate, bits, nchan);
}

void audio_pause(void)
{
	drv.pause();
}

void audio_resume(void)
{
	drv.cont();
}

void audio_stop(void)
{
	drv.stop();
}

void audio_volume(int vol)
{
	drv.volume(vol);
}

int audio_isplaying(void)
{
	return drv.isplaying();
}
