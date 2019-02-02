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
