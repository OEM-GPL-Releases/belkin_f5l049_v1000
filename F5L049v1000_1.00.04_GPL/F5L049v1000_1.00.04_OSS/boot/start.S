/**
 * @file
 *
 * Copy kernel image from ROM to RAM.
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define __ASSEMBLY__

#include <linux/autoconf.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>

/*
 * Copy COMPRESSED KERNEL into RAM
 */
.section .init
LEAF(copy_kernel_block)
	.set        noreorder
	la          t0, _kernel_begin
	la          t1, _kernel_end
	la          t2, _start_ram_addr
	lw          s0, (t0)
	li          s1, 0xffffffff
	bne         s0, s1, copy_kernel
	nop

/*
 *  Not found KERNEL on the flash ROM.
 */
1:
	b           1b
	nop

copy_kernel:
	beq         t0, t1, copy_kernel_end
	nop
	lw          s0, 0(t0)
	sw          s0, 0(t2)
	addiu       t0, t0, 4
	addiu       t2, t2, 4
	b           copy_kernel
	nop
copy_kernel_end:


/*
 * All Cache Clear
 */
cache_clear:
	la          t0, 0x80000000
	li          t1, 0x10000
2:
	cache       Index_Invalidate_I, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 2b
	nop
	la          t0, 0x80000000
	li          t1, 0x8000
3:	cache       Index_Writeback_Inv_D, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 3b
	nop

	/* Jump to uncompress routine */
	la          t0, _start_ram_addr
	jr          t0
	nop


END(copy_kernel_block)
