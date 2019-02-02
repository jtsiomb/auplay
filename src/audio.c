#include <stdio.h>
#include "audio.h"
#include "audrv.h"

static struct audrv drv;

static audio_callback_func cbfunc;
static void *cbcls;

/* driver detect/init functions are defined in their respective source files */
int sb_detect(struct audrv *drv);


int audio_init(void)
{
	if(sb_detect(&drv)) {
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

void audio_setvolume(int ctl, int vol)
{
	drv.setvolume(ctl, vol);
}

int audio_getvolume(int ctl)
{
	return drv.getvolume(ctl);
}

int audio_isplaying(void)
{
	return drv.isplaying();
}
