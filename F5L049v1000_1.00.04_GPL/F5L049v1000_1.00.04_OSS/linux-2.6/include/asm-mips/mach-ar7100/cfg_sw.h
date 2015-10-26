/**
 * @file
 *
 * Push Switch control config
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _CFG_SW_H
#define _CFG_SW_H	1

#include <asm/mach-ar7100/ar7100.h>

#define SX_SW_READ(reg)				ar7100_reg_rd(reg)
#define SX_SW_WRITE(reg, val)		ar7100_reg_wr_nf(reg, val)

/* Local Defines */
#define SW_IRQ_DISABLE		0xFFFFFFFF

/* How many SWs */
#define SW_NUM				3

/* SW BIT */
#define SW_P1_MASK			0x00001000L
#define SW_P2_MASK			0x00200000L
#define SW_P3_MASK			0x00004000L

/* ON DATA */
#define SW_P1_ON			0x00000000L
#define SW_P2_ON			0x00000000L
#define SW_P3_ON			SW_P3_MASK

/* OFF DATA */
#define SW_P1_OFF			SW_P1_MASK
#define SW_P2_OFF			SW_P2_MASK
#define SW_P3_OFF			0x00000000L

/* IRQ Number */
#define SW_P1_IRQNUM		AR7100_GPIO_IRQn(12)
#define SW_P2_IRQNUM		AR7100_GPIO_IRQn(21)
#define SW_P3_IRQNUM		SW_IRQ_DISABLE

/* SW PORT */
#define SW_OUT_PORT			AR7100_GPIO_IN
#define SW_IN_PORT			AR7100_GPIO_IN

/* GPIO INPUT/OUTPUT MODE setting register */
#define SW_GPIO_CTL			AR7100_GPIO_OE

/* GPIO ENABLE INPUT is low or high */
/* low = 0
 * high = 1
 */
#define SW_GPIO_ENABLE_INPUT 0

#endif /* _CFG_SW_H */
