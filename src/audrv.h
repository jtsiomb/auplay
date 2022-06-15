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
#ifndef AUDIO_DRIVER_H_
#define AUDIO_DRIVER_H_

struct audrv {
	void (*start)(int rate, int bits, int nchan);
	void (*pause)(void);
	void (*cont)(void);
	void (*stop)(void);
	void (*setvolume)(int ctl, int vol);
	int (*getvolume)(int ctl);
	int (*isplaying)(void);
};

#endif	/* AUDIO_DRIVER_H_ */
