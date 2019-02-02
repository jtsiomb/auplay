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
