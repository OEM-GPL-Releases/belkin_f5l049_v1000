/**
 * @file
 *
 * LED control config
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _CFG_LED_H
#define _CFG_LED_H	1

/* How many LEDs */
#include <asm/mach-ar7100/ar7100.h>

#define SX_LED_READ(reg)			ar7100_reg_rd(reg)
#define SX_LED_WRITE(reg, val)		ar7100_reg_wr_nf(reg, val)

#define LED_NUM				12

/* LED No. */
#define LED_USB1			1 /* P1 */
#define LED_USB2			2 /* P2 */
#define LED_USB3			3 /* P3 */
#define LED_USB4			4 /* P4 */
#define LED_RJ_STATUS			5 /* P5 */
#define LED_WPS				6 /* P6 */
#define LED_WIRELESS			7 /* P7 */
#define LED_RJ_LINK			8 /* P8 */
#define LED_POWER			9 /* P9 */
#define USB_5V_EN			10 /* P10 */
#define USB_PWR_SW			11 /* P11 */
#define USB_OVCUR			12 /* P12 */

/* LED BIT */
#define LED_P1_MASK			0x00000001 /* 1: 000000000001 */
#define LED_P2_MASK			0x00000002 /* 2: 000000000010 */
#define LED_P3_MASK			0x00000008 /* 3: 000000001000 */
#define LED_P4_MASK			0x00000010 /* 4: 000000010000 */
#define LED_P5_MASK			0x00000020 /* 5: 000000100000 */
#define LED_P6_MASK			0x00000040 /* 6: 000001000000 */
#define LED_P7_MASK			0x00000080 /* 7: 000010000000 */
#define LED_P8_MASK			0x00000100 /* 8: 000100000000 */
#define LED_P9_MASK			0x00000800 /* 9: 100000000000 */
#define LED_P10_MASK		0x00020000
#define LED_P11_MASK		0x00008000
#define LED_P12_MASK		0x00010000

/* ON DATA */
#define LED_P1_ON			0x00000000L
#define LED_P2_ON			0x00000000L
#define LED_P3_ON			0x00000000L
#define LED_P4_ON			0x00000000L
#define LED_P5_ON			0x00000000L
#define LED_P6_ON			0x00000000L
#define LED_P7_ON			0x00000000L
#define LED_P8_ON			0x00000000L
#define LED_P9_ON			0x00000000L
#define LED_P10_ON			LED_P10_MASK
#define LED_P11_ON			LED_P11_MASK
#define LED_P12_ON			LED_P12_MASK

/* OFF DATA */
#define LED_P1_OFF			LED_P1_MASK
#define LED_P2_OFF			LED_P2_MASK
#define LED_P3_OFF			LED_P3_MASK
#define LED_P4_OFF			LED_P4_MASK
#define LED_P5_OFF			LED_P5_MASK
#define LED_P6_OFF			LED_P6_MASK
#define LED_P7_OFF			LED_P7_MASK
#define LED_P8_OFF			LED_P8_MASK
#define LED_P9_OFF			LED_P9_MASK
#define LED_P10_OFF			0x00000000
#define LED_P11_OFF			0x00000000
#define LED_P12_OFF			0x00000000

/* LED PORT */
#define LED_OUT_PORT		AR7100_GPIO_OUT
#define LED_IN_PORT			AR7100_GPIO_OUT

/* GPIO INPUT/OUTPUT MODE setting register */
#define LED_GPIO_CTL		AR7100_GPIO_OE

/* GPIO ENABLE OUTPUT is low or high */
/* low = 0
 * high = 1
 */
#define LED_GPIO_ENABLE_OUTPUT     1
#endif /* _CFG_LED_H */
