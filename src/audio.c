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
