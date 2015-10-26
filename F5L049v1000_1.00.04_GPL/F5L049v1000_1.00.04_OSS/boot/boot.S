/**
 * @file
 *
 * Initialize hardware and firmware information
 * 
 * Copyright (C) 2008 -2009 silex technology, Inc.
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

#define RAM_CACHEADDR_BASE     0x80000000

#define CFG_ICACHE_SIZE        0x10000
#define CFG_DCACHE_SIZE        0x8000
#define CFG_CACHELINE_SIZE     32

LEAF(startup_comp)
	.set        noreorder
	nop

/*
 * hold the check sum number at 0x08 offset of startup_comp.
 *
 */
	.align      3
Check_Sum:
	.hword      0
	.align      3

/*
 * ??
 */
	mtc0        zero, CP0_TAGLO

/*
 * Cache init (Store Tag)
 */
	li          t1, CFG_ICACHE_SIZE
	li          t2, CFG_DCACHE_SIZE
	li          t3, CFG_CACHELINE_SIZE

	li          t0, RAM_CACHEADDR_BASE
	add         t1, t1, t0
icache_loop:
	cache       Index_Store_Tag_I, 0(t0)
	add         t0, t0, t3
	bne         t0, t1, icache_loop
	nop

	li          t0, RAM_CACHEADDR_BASE
	add         t2, t2, t0
dcache_loop:
	cache       Index_Store_Tag_D, 0(t0)
	add         t0, t0, t3
	bne         t0, t2, dcache_loop
	nop

/*
 * All Cache Clear
 */
	la          t0, 0x80000000
	li          t1, 0x10000
1:
	cache       Index_Invalidate_I, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 1b
	nop
	la          t0, 0x80000000
	li          t1, 0x8000
2:	cache       Index_Writeback_Inv_D, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 2b
	nop

/*
 * Cream illegal operation.
 * Disable timer.
 */
	la          t1, 0xb8060014
	sw          zero, 0(t1)

/*
 * CP0 Config Register
 * Write through Disable(default=Enable), Cacheable
 */
	li          t1, 0x3
	mtc0        t1, CP0_CONFIG

/*
 * CP0 Status Register
 * Interrupt vector address change from ROM addr to RAM addr.
 */
	li          t0, 0x10000004
	mtc0        t0, CP0_STATUS

/*
 * Cause register clear
 */
	mtc0        zero, CP0_CAUSE
	nop

/*
 * Transer to expand code
 */
	la          t0, _init_start
	la          t1, _init_end
	la          t2, _start_init_ram_addr
copy_init:
	beq         t0, t1, copy_init_end
	nop
	lw          s0, 0(t0)
	sw          s0, 0(t2)
	addiu       t0, t0, 4
	addiu       t2, t2, 4
	b           copy_init
	nop
copy_init_end:


/*
 * All Cache Clear
 */
	la          t0, 0x80000000
	li          t1, 0x10000
3:
	cache       Index_Invalidate_I, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 3b
	nop
	la          t0, 0x80000000
	li          t1, 0x8000
4:	cache       Index_Writeback_Inv_D, 0(t0)
	nop
	addiu       t1, t1, -32
	addiu       t0, t0, 32
	bnez        t1, 4b
	nop

	la          t0, copy_kernel_block
	jr          t0
	nop
END(startup_comp)


/* ------------------------------------------------------------------------- */
/* Firmware information                                                      */
/* ------------------------------------------------------------------------- */
/*	.set noreorder */
	.org 0x0400,0xff

	.globl ROM_CheckSum_Boot
ROM_CheckSum_Boot:
	.hword	0xFFFF

/* ------------------------------------------------------------------------- */
	.balign 0x10
	.globl SeriesName_Boot
SeriesName_Boot:
/* --------------123456789012345 */
    .ascii      "Belkin","\0"

/* ------------------------------------------------------------------------- */
	.balign 0x10
	.globl MachineType_Boot
MachineType_Boot:
/* --------------123456789012345 */
    .ascii      "Home Base","\0"

/* ------------------------------------------------------------------------- */
	.balign 0x10
	.globl Version_Boot
Version_Boot:
/* --------------123456789012345 */
    .ascii      "1.00.04","\0"
