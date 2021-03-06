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
#include "dpmi.h"

void dpmi_real_int(int inum, struct dpmi_real_regs *regs)
{
	unsigned char int_num = (unsigned char)inum;
	__asm {
		mov eax, 0x300
		mov edi, regs
		mov bl, int_num
		mov bh, 0
		xor ecx, ecx
		int 0x31
	}
}

void *dpmi_mmap(uint32_t phys_addr, unsigned int size)
{
	uint16_t mem_high, mem_low;
	uint16_t phys_high = phys_addr >> 16;
	uint16_t phys_low = phys_addr & 0xffff;
	uint16_t size_high = size >> 16;
	uint16_t size_low = size & 0xffff;
	unsigned int err, res = 0;

	__asm {
		mov eax, 0x800
		mov bx, phys_high
		mov cx, phys_low
		mov si, size_high
		mov di, size_low
		int 0x31
		add res, 1
		mov err, eax
		mov mem_high, bx
		mov mem_low, cx
	}

	if(res == 2) {
		return 0;
	}
	return (void*)(((uint32_t)mem_high << 16) | ((uint32_t)mem_low));
}

void dpmi_munmap(void *addr)
{
	uint16_t mem_high = (uint32_t)addr >> 16;
	uint16_t mem_low = (uint16_t)addr;

	__asm {
		mov eax, 0x801
		mov bx, mem_high
		mov cx, mem_low
		int 0x31
	}
}
