/**
 * @file
 *
 * LED control API
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _LED_CTRL_H
#define _LED_CTRL_H 1

#define LED_ON		1
#define LED_OFF		0

#define LED_PATTERN_ON		0x0001
#define LED_PATTERN_OFF		0x0000
#define LED_PATTERN_1SHOT	0x0001
#define LED_PATTERN_BLINK	0x0001

#define LED_DEF_INTVL_1SHOT		50	/* 50msec */
#define LED_DEF_INTVL_BLINK		200	/* 200msec */

#define LED_PATTERN_BITLEN_ON		1
#define LED_PATTERN_BITLEN_OFF		1
#define LED_PATTERN_BITLEN_1SHOT	2
#define LED_PATTERN_BITLEN_BLINK	2

/* Continue types ----------------------------------------------------------- */
enum tag_LED_CTRL_CTYPE {
	LED_CTRL_CTYPE_NONE = 0,
	LED_CTRL_CTYPE_TIME,		/* Time         */
	LED_CTRL_CTYPE_COUNT,		/* Count        */
	LED_CTRL_CTYPE_PERMANENT	/* Parmanet     */
};

/* LED Control options ----------------------------------------------------- */
#define	LED_OPT_LOCK	0x0001	/* Locked LED State         */
#define	LED_OPT_FORCE	0x0002	/* Forcibly set LED pattern */


/* LED Control pattern parameters  ----------------------------------------- */
struct led_ctl_pattern {
	u32 interval;			/* exec blink Interval         */

	u32 pattern;			/* blink pattern               */
	u32 pattern_bitlen;		/* blink pattern bit length    */

	u32 cont_type;			/* Continue type (Times/msec)  */
	s32 cont_volume;		/* Continue volume (Times/msec */

	u32 after_ctrl;			/* After control (ON/OFF)      */
};

int sx_led_on(int led_num, u32 option);
int sx_led_off(int led_num, u32 option);
int sx_led_ctl_pattern(int led_num, struct led_ctl_pattern *ptn, u32 option);
int sx_led_read(int led_num);

#endif /* _LED_CTRL_H */
