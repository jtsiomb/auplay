#ifndef AU_SB_H_
#define AU_SB_H_

/* returns true (nonzero) if a sound blaster DSP is detected in the ISA bus
 * and sets the internal base_port so that subsequent calls can find it
 */
int sb_detect(void);

/* returns 0 for success, non-zero if the DSP isn't responding at the currently
 * selected base port
 */
int sb_reset_dsp(void);

void *sb_buffer(int *size);

void sb_set_output_rate(int rate);

void sb_start(int rate, int bits, int nchan);
void sb_pause(void);
void sb_continue(void);
void sb_stop(void);
int sb_isplaying(void);

void sb_volume(int vol);

#endif	/* AU_SB_H_ */
