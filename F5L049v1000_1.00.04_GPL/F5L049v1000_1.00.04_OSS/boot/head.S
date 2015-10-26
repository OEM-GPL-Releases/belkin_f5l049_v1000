/**
 * @file
 *
 * Uncompress kernel image
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
#include <linux/init.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>

#include "kernelentry.h"

/*
 *   Setting Initialization check starts, if t9 == 2.
 */
.text
LEAF(head_comp)
	.set        noreorder

	la          t0, stack_start
	lw          sp, (t0)
	nop

/*
 * Clear BSS
 */
	la          t0, _edata
	sw          zero, (t0)
	la          t1, _end
	li          t2, 0xfffffff8
	and         t1, t1, t2
1:
	addiu       t0, 4
	sw          zero, (t0)
	bne         t0, t1, 1b
    nop

/*
 * Shift stack pointer 4 words
 */
	addiu       sp, -16


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

/*
 * Do the decompression, and jump to the new kernel..
 */
decompress:
	jal         decompress_kernel
	nop

/*
 * Data Cache WriteBack Again
 */
	la          t0, 0x80000000
	li          t1, 0x8000
14:
	cache       Index_Writeback_Inv_D, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 14b
	nop

/* We want to jump kernel_entry function, so we are using script.sh 
 * to read kernel_entry label from System.map.
 */
	la          t0, KERNELENTRY
	jalr        t0
	nop
15:
	j		15b
	nop
	END(head_comp)
