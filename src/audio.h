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
#ifndef AUDIO_H_
#define AUDIO_H_

/* mixer channels */
enum {
	AUDIO_DEFAULT,
	AUDIO_MASTER,
	AUDIO_PCM
};

typedef int (*audio_callback_func)(void *buffer, int size, void *cls);

int audio_init(void);

void audio_set_callback(audio_callback_func func, void *cls);
int audio_callback(void *buf, int sz);

void audio_play(int rate, int bits, int nchan);
void audio_pause(void);
void audio_resume(void);
void audio_stop(void);
int audio_isplaying(void);

/* audio volume: 0-255 */
void audio_setvolume(int ctl, int vol);
int audio_getvolume(int ctl);

#endif	/* AUDIO_H_ */
