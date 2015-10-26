/**
 * @file
 *
 * LED control API
 * 
 * Copyright (C) 2008 -2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/module.h>

#include "led_ctrl.h"
#include "cfg_led.h"

#define CYCLE  (1000 / HZ)			/* 1 Tick time(msec) */

struct led_ctrl {
	int led_num;					/* LED Number                          */
	int locked;						/* LED locked(1)/unlocked(0)           */
	struct timer_list *timer;		/* LED Blinking timer                  */
	int shift_cnt;					/* Shift Pattern count                 */

	u32 port_out;					/* LED write register                  */
	u32 port_in;					/* LED read  register                  */
	u32 port_mask;					/* LED BIT MASK                        */

	u32 on_data;					/* LED turn on register pattern        */
	u32 off_data;					/* LED turn off register pattern       */
	u32 status;						/* Current LED status                  */

	struct led_ctl_pattern ptn;		/* Current pattern                     */
};

static struct led_ctrl led_ctrl_tbl[LED_NUM] = {
	{
		1, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P1_MASK,
		LED_P1_ON,    LED_P1_OFF,  0,
		{0}
	},
#if LED_NUM >= 2
	{
		2, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P2_MASK,
		LED_P2_ON,    LED_P2_OFF,  0,
		{0}
	},
#if LED_NUM >= 3
	{
		3, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P3_MASK,
		LED_P3_ON,    LED_P3_OFF,  0,
		{0}
	},
#if LED_NUM >= 4
	{
		4, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P4_MASK,
		LED_P4_ON,    LED_P4_OFF,  0,
		{0}
	},
#if LED_NUM >= 5
	{
		5, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P5_MASK,
		LED_P5_ON,    LED_P5_OFF,  0,
		{0}
	},
#if LED_NUM >= 6
	{
		6, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P6_MASK,
		LED_P6_ON,    LED_P6_OFF,  0,
		{0}
	},
#if LED_NUM >= 7
	{
		7, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P7_MASK,
		LED_P7_ON,    LED_P7_OFF,  0,
		{0}
	},
#if LED_NUM >= 8
	{
		8, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P8_MASK,
		LED_P8_ON,    LED_P8_OFF,  0,
		{0}
	},
#if LED_NUM >= 9
	{
		9, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P9_MASK,
		LED_P9_ON,    LED_P9_OFF,  0,
		{0}
	},
#if LED_NUM >= 10
	{
		10, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P10_MASK,
		LED_P10_ON,    LED_P10_OFF,  0,
		{0}
	},
#if LED_NUM >= 11
	{
		11, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P11_MASK,
		LED_P11_ON,    LED_P11_OFF,  0,
		{0}
	},
#if LED_NUM >= 12
	{
		12, 0, NULL, 0,
		LED_OUT_PORT, LED_IN_PORT, LED_P12_MASK,
		LED_P12_ON,    LED_P12_OFF,  0,
		{0}
	},
#endif /* LED_NUM >= 12 */
#endif /* LED_NUM >= 11 */
#endif /* LED_NUM >= 10 */
#endif /* LED_NUM >= 9 */
#endif /* LED_NUM >= 8 */
#endif /* LED_NUM >= 7 */
#endif /* LED_NUM >= 6 */
#endif /* LED_NUM >= 5 */
#endif /* LED_NUM >= 4 */
#endif /* LED_NUM >= 3 */
#endif /* LED_NUM >= 2 */
};

static spinlock_t led_spnlock;

/**
 * LED lock.
 *
 * @param ledc Target LED
 *
 */
static void led_lock(struct led_ctrl *ledc)
{
	if(ledc != NULL)	ledc->locked = 1;
}

/**
 * LED unlock.
 *
 * @param ledc Target LED
 *
 */
static void led_unlock(struct led_ctrl *ledc)
{
	if(ledc != NULL)	ledc->locked = 0;
}

/**
 * Set LED turn on / off
 *
 * @param ledc Target LED
 * @parm on_off
 *
 */
static void led_set(struct led_ctrl *ledc, int on_off)
{
	u32 data;

	data = SX_LED_READ(ledc->port_in);
	data &= ~(ledc->port_mask);
	data |= (on_off == LED_OFF) ? ledc->off_data : ledc->on_data;

	SX_LED_WRITE(ledc->port_out, data);

	/* Save current LED Status */
	ledc->status = on_off;
}

/**
 * Get LED status (On or Off)
 *
 * @param ledc Target LED
 *
 * @return LED_OFF ON
 * @return LED_OFF OFF
 */
static int led_get_status(struct led_ctrl *ledc)
{
	u32 data;

	data = SX_LED_READ(ledc->port_in) & ledc->port_mask;

	return (data == ledc->on_data) ? LED_ON : LED_OFF;
}

/**
 * Finish the LED pattern control.
 *
 * @param ledc Target LED
 * 
 * @return 0 success
 * @return <0 error
 */
static int led_finish_pattern(struct led_ctrl *ledc)
{
	/* Suspend timer */
	del_timer_sync(ledc->timer);
	
	/* After control process start! */
	led_unlock(ledc);

	return 0;
}

/**
 * LED Timer function.
 * This function called by timer.
 * 
 * @parm arg  timer argument (Target LED info)
 *
 */
static void led_timer_fn(unsigned long arg)
{
	int on_off;
	int shift_cnt;
	unsigned long j = jiffies;
	struct led_ctrl *ledc = (struct led_ctrl*)arg;
	struct led_ctl_pattern *ptn = &ledc->ptn;
	unsigned long flags;

	spin_lock_irqsave(&led_spnlock, flags);

	/* Blink process */
	shift_cnt = ledc->shift_cnt;
	if (shift_cnt >= ptn->pattern_bitlen)
		shift_cnt = 0;

	ledc->shift_cnt = shift_cnt + 1;

	on_off = (((ptn->pattern >> shift_cnt) & 0x0001) != 0) ? LED_ON : LED_OFF;
	if (ledc->status != on_off)
		led_set(ledc, on_off);

	/* cut down cont_volume according to cont_type */
	switch(ptn->cont_type) {
	case LED_CTRL_CTYPE_TIME:
		ptn->cont_volume -= ptn->interval;
		break;

	case LED_CTRL_CTYPE_COUNT:
		if(ledc->shift_cnt >= ptn->pattern_bitlen) {
			/* revolve pattern */
			ptn->cont_volume--;
		}
		break;
	}

	/* Check finish state */
	if(ptn->cont_volume < 0) {
		/* Stop timer and set LED finish state */
		led_finish_pattern(ledc);

		if(ledc->status != ptn->after_ctrl)
			led_set(ledc, ptn->after_ctrl);

		spin_unlock_irqrestore(&led_spnlock, flags);
		return;
	}

	/* continue pattern */
	ledc->timer->data = (unsigned long)ledc;
	ledc->timer->function = led_timer_fn;
	ledc->timer->expires = j + ptn->interval;

	spin_unlock_irqrestore(&led_spnlock, flags);

	add_timer(ledc->timer);
}

/**
 * Execute blinking according to LED pattern.
 *
 * This function startup timer.
 * 
 * @parm ledc Target LED
 * 
 * @return 0  Success
 * @return <0 Error
 */
static int led_exec_pattern(struct led_ctrl *ledc)
{
	ledc->shift_cnt = 0;

	/* Immediate start up Timer */
	ledc->timer->data = (unsigned long)ledc;
	ledc->timer->function = led_timer_fn;
	ledc->timer->expires = jiffies;
	add_timer(ledc->timer);

	return 0;
}

/** 
 * Internal function for sx_led_on/off.
 *
 * @parm led_num Target LED number
 * @parm on_off  Turn on/off?
 * @parm option Mode
 *
 * @return 0  Success
 * @return <0 Error
 */
static int led_on_off_internal(int led_num, int on_off, u32 option)
{
	struct led_ctrl *ledc;
	struct led_ctl_pattern *ptn;
	unsigned long flags;

	if(led_num < 1 || led_num > LED_NUM)
		return -1;

	ledc = &led_ctrl_tbl[led_num-1];
	ptn = &ledc->ptn;

	spin_lock_irqsave(&led_spnlock, flags);

	/* if LED had already locked ;-( */
	if(ledc->locked && !(option & LED_OPT_FORCE)) {
		if (ptn->cont_type != LED_CTRL_CTYPE_PERMANENT) {
			/* Illegal specification!! */
			/* Overwrite Next pattern setting */
			ptn->after_ctrl = on_off;
			spin_unlock_irqrestore(&led_spnlock, flags);
			return -1;
		}
		else {
			spin_unlock_irqrestore(&led_spnlock, flags);
			return -1;
		}
	}

	led_finish_pattern(ledc);

	ptn->interval        = 0;
	ptn->pattern         = (on_off == LED_ON) ? LED_PATTERN_ON : LED_PATTERN_OFF;
	ptn->pattern_bitlen  = 1;
	ptn->cont_type       = LED_CTRL_CTYPE_PERMANENT;
	ptn->cont_volume     = 0x0FFFFFFF; /* Dummy Data */
	ptn->after_ctrl      = on_off;

	if(ledc->status != on_off) {
		/* Change LED Turn on / off */
		led_set(ledc, on_off);
	}

	if(option & LED_OPT_LOCK)
		led_lock(ledc);

	spin_unlock_irqrestore(&led_spnlock, flags);

	return 0;
}

/** 
 * Turn on LED.
 *
 * @parm led_num Target LED number
 * @parm option Mode
 *
 * @return 0  Success
 * @return <0 Error
 */
int sx_led_on(int led_num, u32 option)
{
	return led_on_off_internal(led_num, LED_ON, option);
}
EXPORT_SYMBOL(sx_led_on);


/** 
 * Turn off LED.
 *
 * @parm led_num Target LED number
 * @parm option Mode
 *
 * @return 0  Success
 * @return <0 Error
 */
int sx_led_off(int led_num, u32 option)
{
	return led_on_off_internal(led_num, LED_OFF, option);

}
EXPORT_SYMBOL(sx_led_off);

/** 
 * Control LED turn on/off according to pattern.
 *
 * @parm ledc Target LED
 * @parm ptn Control pattern info
 * @parm option Mode
 *
 * @return 0  Success
 * @return <0 Error
 */
int sx_led_ctl_pattern(int led_num, struct led_ctl_pattern *ptn, u32 option)
{
	struct led_ctrl *ledc;
	u32 interval, volume;
	unsigned long flags;

	if(led_num < 1 || led_num > LED_NUM)
		return -EINVAL;

	ledc = &led_ctrl_tbl[led_num-1];

	/* MAX Check */
	if((ptn->pattern_bitlen == 0) ||
			 (ptn->pattern_bitlen > sizeof(ptn->pattern) * 8)) {
		return -EINVAL;
	}

	/* interval (msec) -> jiffies */
	interval = ptn->interval / CYCLE;
	if(interval == 0)
		return -EINVAL;

	/* Change cont_volume according to continue type */
	switch(ptn->cont_type) {
	case LED_CTRL_CTYPE_TIME:
		volume = ptn->cont_volume / CYCLE;
		break;

	case LED_CTRL_CTYPE_COUNT:
		volume = ptn->cont_volume;
		break;

	case LED_CTRL_CTYPE_PERMANENT:
		volume = 0x0FFFFFFF; /* Dummy data */
		break;

	default:
		return -EINVAL;
	}

	spin_lock_irqsave(&led_spnlock, flags);

	/* if LED had already locked ;-( */
	if((ledc->locked) && !(option & LED_OPT_FORCE)) {
		spin_unlock_irqrestore(&led_spnlock, flags);
		return -EBUSY;
	}

	led_finish_pattern(ledc);

	/* Current pattern setting */
	ledc->ptn.interval = interval;
	ledc->ptn.pattern = ptn->pattern;
	ledc->ptn.pattern_bitlen = ptn->pattern_bitlen;
	ledc->ptn.cont_type = ptn->cont_type;
	ledc->ptn.cont_volume = volume;
	ledc->ptn.after_ctrl = ptn->after_ctrl;

	/* !!!! Execute !!!! */
	led_exec_pattern(ledc);

	/* If this opration request LOCKing LED, reserved led resource. */
	if(option & LED_OPT_LOCK)
		led_lock(ledc);

	spin_unlock_irqrestore(&led_spnlock, flags);

	return 0;
}
EXPORT_SYMBOL(sx_led_ctl_pattern);

/** 
 * Get LED status.
 * 
 * @parm led_num Target LED number
 *
 * @return 0 Turn on
 * @return 1 Turn off
 */
int sx_led_read(int led_num)
{
	struct led_ctrl *ledc;
	int ret;
	unsigned long flags;

	if(led_num < 1 || led_num > LED_NUM)
		return -1;

	ledc = &led_ctrl_tbl[led_num-1];

	spin_lock_irqsave(&led_spnlock, flags);
	ret = led_get_status(ledc);
	spin_unlock_irqrestore(&led_spnlock, flags);
	
	return ret;
}
EXPORT_SYMBOL(sx_led_read);

/** 
 * Initial LED control.
 * 
 * 
 * @return 0 success
 * @return <0 error
 */
static int __init sx_led_init(void)
{
	int i;
	struct led_ctrl *ledc;

	for(i = 0; i < LED_NUM; i++) {
		ledc = &led_ctrl_tbl[i];

		ledc->timer = kmalloc(sizeof(struct timer_list), GFP_KERNEL);
		if(ledc->timer == NULL) {
			printk(KERN_ERR "Error: no mem in sx_led_init()\n");
			return -1;
		}

		init_timer(ledc->timer);

		/* Initial status */
		/* Because LED output initializing do boot loader or Linux early
		 * initializing not this feature.
		 */
		ledc->status = led_get_status(ledc);

		/* Initialize led parameter */
		memset(&ledc->ptn, 0, sizeof(struct led_ctl_pattern));

		led_unlock(ledc);
	}

	/* lock for led_ctrl_tbl */
	spin_lock_init(&led_spnlock);

	return 0;
}

/** 
 * Cleanup LED control.
 * 
 */
static void __exit sx_led_cleanup(void)
{
	int i;
	struct led_ctrl *ledc;

	ledc = led_ctrl_tbl;

	for(i=0; i<LED_NUM; i++) {
		if(ledc->timer != NULL) {

			if(ledc->timer->function != NULL)
				del_timer_sync(ledc->timer);

			kfree(ledc->timer);
		}
	}
}

module_init(sx_led_init);
module_exit(sx_led_cleanup);

