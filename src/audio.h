#ifndef AUDIO_H_
#define AUDIO_H_

typedef int (*audio_callback_func)(void *buffer, int size, void *cls);

void audio_init(void);

void audio_set_callback(audio_callback_func func, void *cls);
int audio_callback(void *buf, int sz);

void audio_play(int rate, int bits, int nchan);
void audio_pause(void);
void audio_resume(void);
void audio_stop(void);
int audio_isplaying(void);

void audio_volume(int vol);

#endif	/* AUDIO_H_ */
