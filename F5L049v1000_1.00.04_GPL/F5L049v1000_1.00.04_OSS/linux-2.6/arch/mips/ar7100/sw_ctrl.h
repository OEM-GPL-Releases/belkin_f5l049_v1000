/**
 * @file
 *
 * Switch control API
 * 
 * Copyright (C) 2008 -2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _SW_CTRL_H
#define _SW_CTRL_H 1

#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#define SW_ON		1
#define SW_OFF		0
/*
 * GPIO interrupt stuff
 */
typedef enum {
	INT_TYPE_EDGE,
	INT_TYPE_LEVEL,
} ar7100_gpio_int_type_t;

typedef enum {
	INT_POL_ACTIVE_LOW,
	INT_POL_ACTIVE_HIGH,
} ar7100_gpio_int_pol_t;

#define SX_SW_SWITCH_UP       0
#define SX_SW_SWITCH_DOWN     1

/* GPIO IN(e.g Push switch) control structure */
struct sx_sw_ctrl {
	int sw_num;                         /* SW Number                           */

	u32 port_in;                        /* SW IN register                      */
	u32 port_mask;                      /* SW BIT MASK                         */

	u32 on_data;                        /* SW ON register pattern              */
	u32 off_data;                       /* SW OFF register pattern             */

	spinlock_t irq_lock;                /* spin_lock for irq and work_queue    */
	unsigned int irq_num;               /* IRQ number for request_irq()        */
	int irq_status;                     /* Push Button Status for interrupt    */
	unsigned long irq_pushed_jiffies;   /* Push switch pushed current jiffies(tick) */
	struct work_struct irq_wq;          /* work queue for Push switch irq */
	struct list_head irq_list;          /* list for work queue                 */
	void (*irq_exec)(int, int, unsigned long); /* execute action for bottom half */
};

struct sx_sw_job {
	struct list_head list;
	int action;
	unsigned long msec;
};

#define sx_sw_lock_event()   spin_lock(&sw_event.lock)
#define sx_sw_unlock_event() spin_unlock(&sw_event.lock)

int sx_sw_read(int sw_num);
int sx_sw_set_event(void (*exec)(int, int, unsigned long));

#endif /* _SW_CTRL_H */
