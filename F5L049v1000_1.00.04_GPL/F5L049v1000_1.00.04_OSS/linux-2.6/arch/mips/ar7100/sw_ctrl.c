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

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "ar7100.h"
#include "sw_ctrl.h"
#include "cfg_sw.h"

static struct sx_sw_ctrl sw_ctrl_tbl[SW_NUM] = {
	{
		.sw_num         = 1,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P1_MASK,
		.on_data        = SW_P1_ON,
		.off_data       = SW_P1_OFF,
		.irq_num        = SW_P1_IRQNUM,
	},
#if SW_NUM >= 2
	{
		.sw_num         = 2,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P2_MASK,
		.on_data        = SW_P2_ON,
		.off_data       = SW_P2_OFF,
		.irq_num        = SW_P2_IRQNUM,
	},
#if SW_NUM >= 3
	{
		.sw_num         = 3,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P3_MASK,
		.on_data        = SW_P3_ON,
		.off_data       = SW_P3_OFF,
		.irq_num        = SW_P3_IRQNUM,
	},
#if SW_NUM >= 4
	{
		.sw_num         = 4,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P4_MASK,
		.on_data        = SW_P4_ON,
		.off_data       = SW_P4_OFF,
		.irq_num        = SW_P4_IRQNUM,
	},
#if SW_NUM >= 5
	{
		.sw_num         = 5,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P5_MASK,
		.on_data        = SW_P5_ON,
		.off_data       = SW_P5_OFF,
		.irq_num        = SW_P5_IRQNUM,
	},
#if SW_NUM >= 6
	{
		.sw_num         = 6,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P6_MASK,
		.on_data        = SW_P6_ON,
		.off_data       = SW_P6_OFF,
		.irq_num        = SW_P6_IRQNUM,
	},
#if SW_NUM >= 7
	{
		.sw_num         = 7,
		.port_in        = SW_IN_PORT,
		.port_mask      = SW_P7_MASK,
		.on_data        = SW_P7_ON,
		.off_data       = SW_P7_OFF,
		.irq_num        = SW_P7_IRQNUM,
	},
#endif /* SW_NUM >= 7 */
#endif /* SW_NUM >= 6 */
#endif /* SW_NUM >= 5 */
#endif /* SW_NUM >= 4 */
#endif /* SW_NUM >= 3 */
#endif /* SW_NUM >= 2 */
};

void ar7100_gpio_config_int(int irq_num, 
				ar7100_gpio_int_type_t type, ar7100_gpio_int_pol_t polarity)
{
	u32 val;
	int portnum = irq_num - AR7100_GPIO_IRQ_BASE;

	/*
	 * allow edge sensitive/rising edge too
	 */
	 
	if (type == INT_TYPE_LEVEL) {
		/* level sensitive */
		ar7100_reg_rmw_set(AR7100_GPIO_INT_TYPE, (1 << portnum));
	}
	else {
		/* edge triggered */
		val = ar7100_reg_rd(AR7100_GPIO_INT_TYPE);
		val &= ~(1 << portnum);
		ar7100_reg_wr(AR7100_GPIO_INT_TYPE, val);
	}

	if (polarity == INT_POL_ACTIVE_HIGH) {
		ar7100_reg_rmw_set(AR7100_GPIO_INT_POLARITY, (1 << portnum));
	}
	else {
		val = ar7100_reg_rd(AR7100_GPIO_INT_POLARITY);
		val &= ~(1 << portnum);
		ar7100_reg_wr(AR7100_GPIO_INT_POLARITY, val);
	}

	ar7100_reg_rmw_set(AR7100_GPIO_INT_ENABLE, (1 << portnum));
}


/** 
 * Push switch interrupt bottom half.
 * 
 * @parm cpl    
 * @parm priv   Private data set when call sx_sw_irq_request()
 *
 * @return IRQ_HANDLED  Success and accept next irq.
 */
void sx_sw_irq_workqueue(struct work_struct *work)
{
	struct sx_sw_ctrl *swc = container_of(work, struct sx_sw_ctrl, irq_wq);
	struct sx_sw_job *jobp;

	void (*tmp_irq_exec)(int, int, unsigned long);

	while(1) {

		/* == Critical section for list operation == */
		spin_lock(&swc->irq_lock);
		if(list_empty(&swc->irq_list)) {
			spin_unlock(&swc->irq_lock);
			break;
		}
		jobp = list_entry(swc->irq_list.next, struct sx_sw_job, list);
		list_del(swc->irq_list.next);
		tmp_irq_exec = swc->irq_exec;
		spin_unlock(&swc->irq_lock);
		/* == Critical section for list operation end == */

		if(tmp_irq_exec != NULL) {
			tmp_irq_exec(swc->sw_num, jobp->action, jobp->msec);
		}

		kfree(jobp);
	}
}

/** 
 * Push switch interrupt hander.
 * 
 * @parm cpl    
 * @parm priv   Private data set when call sx_sw_irq_request()
 *
 * @return IRQ_HANDLED  Success and accept next irq.
 */
int sx_sw_irq_hander(int cpl, void *priv)
{
	struct sx_sw_ctrl *swc = (struct sx_sw_ctrl*)priv;
	struct sx_sw_job *jobp;

	jobp = (struct sx_sw_job*)kmalloc(sizeof(struct sx_sw_job), GFP_ATOMIC);
	if(jobp == NULL) {
		printk(KERN_ERR "sxsw:No mem\n");
		return IRQ_HANDLED;
	}

	INIT_LIST_HEAD(&jobp->list);


	if (swc->irq_status == SX_SW_SWITCH_DOWN) {
		ar7100_gpio_config_int(swc->irq_num, INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
		swc->irq_status = SX_SW_SWITCH_UP;

		/* [TODO] Dose not consider jiffies overflow ;( */
		jobp->msec = jiffies_to_msecs(jiffies - swc->irq_pushed_jiffies);
		jobp->action = swc->irq_status;
	}
	else {
		ar7100_gpio_config_int(swc->irq_num, INT_TYPE_LEVEL, INT_POL_ACTIVE_HIGH);
		swc->irq_status = SX_SW_SWITCH_DOWN;
		swc->irq_pushed_jiffies = jiffies;

		jobp->msec = 0;
		jobp->action = swc->irq_status;
	}

	/* == Critical section for list operation == */
	spin_lock(&swc->irq_lock);
	list_add_tail(&jobp->list, &swc->irq_list);
	spin_unlock(&swc->irq_lock);
	/* == Critical section for list operation end == */

	schedule_work(&swc->irq_wq);

	return IRQ_HANDLED;
}

/** 
 * Get Switch(GPIO-IN) status.
 * 
 * @parm sw_num Target Switch number
 *
 * @return SW_ON  on
 * @return SW_OFF off
 */
int sx_sw_read(int sw_num)
{
	struct sx_sw_ctrl *swc;
	u32 rdata;

	if(sw_num < 1 || sw_num > SW_NUM)
		return -1;

	swc = &sw_ctrl_tbl[sw_num-1];
	rdata = SX_SW_READ(swc->port_in) & swc->port_mask;
	if(rdata == SW_P1_ON) {
		return SW_ON;
	}

	return SW_OFF;
}

int sx_sw_set_event(void (*exec)(int, int, unsigned long))
{
	struct sx_sw_ctrl *swc = sw_ctrl_tbl;
	int i;

	/* Push switch IRQ initialize */
	for(i=0; i<SW_NUM; i++, swc++) {
		spin_lock(&swc->irq_lock);
		swc->irq_exec = exec;
		spin_unlock(&swc->irq_lock);
	}

	return 0;
}

/** 
 * Initial Switch(GPIO-IN) control.
 * 
 * @return 0 success
 * @return <0 error
 */
static int __init sx_sw_init(void)
{
	struct sx_sw_ctrl *swc = sw_ctrl_tbl;
	int i, ret = 0;
	char irqname[8];

#if 0
	u32 pdr, mask;

	/* GPIO CTL register configuration for Switch(INPUT) */
	for(i=0, mask=0; i<SW_NUM; i++) {
		mask |= swc[i].port_mask;
	}

	/* The GPIO Direction register donot initialize here */
	/* It should be initialize in early init of linux kernel or boot loader */
	pdr = SX_SW_READ(SW_GPIO_CTL);
	if(SW_GPIO_ENABLE_INPUT == 0)
		pdr = pdr & ~mask;
	else
		pdr = pdr | mask;
	SX_SW_WRITE(SW_GPIO_CTL, pdr);
#endif

	irqname[7] = '\0';
	for(i=0, swc = sw_ctrl_tbl; i<SW_NUM; i++, swc++) {
		if(swc->irq_num == SW_IRQ_DISABLE)
			continue;

		snprintf(irqname, 7, "SW%d", swc->irq_num);

		swc->irq_status = SX_SW_SWITCH_UP;
		swc->irq_pushed_jiffies = 0;
		swc->irq_exec = NULL;
		spin_lock_init(&swc->irq_lock);

		ar7100_gpio_config_int(swc->irq_num, INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
		INIT_WORK(&swc->irq_wq, sx_sw_irq_workqueue);
		INIT_LIST_HEAD(&swc->irq_list);

		ret = request_irq(swc->irq_num, sx_sw_irq_hander,
							(IRQF_DISABLED | IRQF_SHARED),
							irqname, swc);
		if(ret < 0) {
			printk(KERN_ERR "SXSW: request_irq fail %d\n", ret);
		}
	}

	return 0;
}

/** 
 * Cleanup Switch(GPIO-IN) control.
 * 
 */
static void __exit sx_sw_cleanup(void)
{
	/* [TODO] Implement cleanup this module */
	printk(KERN_INFO "sw_ctrl cleanup nothing done.");\
}

module_init(sx_sw_init);
module_exit(sx_sw_cleanup);

