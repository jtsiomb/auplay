#ifndef AUFILE_H_
#define AUFILE_H_

struct au_file {
	int rate, bits, chan;
	void *data;
	unsigned long size;

	FILE *fp;
	void (*close)(struct au_file*);
	void (*reset)(struct au_file*);
	int (*read)(struct au_file*, void*, int);
};

struct au_file *au_open(const char *fname);
void au_close(struct au_file *au);

void au_reset(struct au_file *au);
int au_read(struct au_file *au, void *buf, int size);

#endif	/* AUFILE_H_ */
