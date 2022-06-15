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
