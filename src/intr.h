#ifndef INTR_H_
#define INTR_H_

#define INTERRUPT	__interrupt __far

#define IRQ_TO_INTR(x)	((x) + 8)

/* PIC command and data ports */
#define PIC1_CMD	0x20
#define PIC1_DATA	0x21
#define PIC2_CMD	0xa0
#define PIC2_DATA	0xa1
/* PIC operation command word 2 bits */
#define OCW2_EOI	(1 << 5)

unsigned char get_irq_mask(int pic);
#pragma aux get_irq_mask = \
		"cmp bx, 0" \
		"jnz getpic2" \
		"in al, 0x21" \
		"jmp getend" \
		"getpic2:" \
		"in al, 0xa1" \
		"getend:" \
		value[al] parm[ebx];

void mask_irq(int irq);
#pragma aux mask_irq = \
		"mov dx, 0x21" \
		"cmp ax, 8" \
		"jb skip_mask_pic2" \
		"mov dx, 0xa1" \
		"sub ax, 8" \
		"skip_mask_pic2:" \
		"mov cl, al" \
		"mov ah, 1" \
		"shl ah, cl" \
		"in al, dx" \
		"or al, ah" \
		"out dx, al" \
		parm[eax] modify[cl dx];

void unmask_irq(int irq);
#pragma aux unmask_irq = \
		"mov dx, 0x21" \
		"cmp ax, 8" \
		"jb skip_unmask_pic2" \
		"mov dx, 0xa1" \
		"sub ax, 8" \
		"skip_unmask_pic2:" \
		"mov cl, al" \
		"mov ah, 1" \
		"shl ah, cl" \
		"not ah" \
		"in al, dx" \
		"and al, ah" \
		"out dx, al" \
		parm[eax] modify[cl dx];

#endif	/* INTR_H_ */
