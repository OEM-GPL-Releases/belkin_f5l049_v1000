/**
 * @file
 *
 * LED control proc fs
 * 
 * Copyright (C) 2008 -2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include "cfg_led.h"
#include "led_ctrl.h"

#define MAX_BUF_SIZE 128
#define PROC_SILEX_LED_TOP	"silex/led"

static struct proc_dir_entry *led_dirp[LED_NUM];
static char write_tmpbuf[MAX_BUF_SIZE];
static int led_index[LED_NUM];

static struct semaphore led_proc_sem;

/**
 * Read call back function from proc-fs.
 * 
 * @return Created data length.
 * @return <0 error
 */
static int led_proc_read(char *buf, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;
	int *index = (int*)data;

	if ( down_interruptible( &led_proc_sem ) ) {
		printk( KERN_INFO "%s : down_interruptible for read failed\n", __FUNCTION__ );
		return -ERESTARTSYS;
	}

	len += sprintf(buf, "%d\n", sx_led_read(*index));
	up( &led_proc_sem );

	*eof = 1;
	return len;
}

/**
 * Write call back function from proc-fs.
 * 
 * @return Handled data length.
 * @return <0 error
 */
static int led_proc_write(struct file* filp, const char* buf, 
					unsigned long count, void* data)
{
	int ret;
	int *index = (int*)data;
	struct led_ctl_pattern ptn;
	u32 option;

	if(count > MAX_BUF_SIZE)
		return -EINVAL;

	if ( down_interruptible( &led_proc_sem ) ) {
		printk( KERN_INFO "%s : down_interruptible for write failed\n", __FUNCTION__ );
		return -EFAULT;
	}

	if ( copy_from_user( write_tmpbuf, buf, count ) ) {
		up( &led_proc_sem );
		printk( KERN_INFO "%s : copy_from_user failed\n", __FUNCTION__ );
		return -EFAULT;
	}

	/* Interval time */
	ret = sscanf(write_tmpbuf, "%d %x %d %d %d %d %x", 
					&ptn.interval,
					&ptn.pattern,
					&ptn.pattern_bitlen,
					&ptn.cont_type,
					&ptn.cont_volume,
					&ptn.after_ctrl,
					&option);

	/* Writing data is needed 7 items! */
	if(ret != 7) {
		up( &led_proc_sem );
		printk(KERN_ERR "Illegal LED pattern!\n");
		return -EINVAL;
	}

	/* Turn ON/Off permanent */
	if(ptn.pattern_bitlen == 1 && ptn.cont_type == LED_CTRL_CTYPE_PERMANENT) {
		switch(ptn.pattern) {
		case 0:
			sx_led_off((int)*index, option);
			break;
		case 1:
			sx_led_on((int)*index, option);
			break;
		default:
			printk(KERN_ERR "Illegal LED pattern!\n");
			up( &led_proc_sem );
			return -EINVAL;
		}
	}
	/* Pattern execution */
	else {
		ret = sx_led_ctl_pattern((int)*index, &ptn, option);
		if(ret < 0) {
			up( &led_proc_sem );
			printk(KERN_ERR "Could not exec LED pattern! %d\n", ret);
			return ret;
		}
	}

	up( &led_proc_sem );

	return count;

}

/**
 * Initial Switch proc-fs.
 * 
 * @return 0 success
 * @return <0 error
 */
static int __init led_proc_init_module (void)
{
	int i;
	char proc_name[8];
	struct proc_dir_entry *parent;
	struct proc_dir_entry *dirp = led_dirp[0];

	parent = proc_mkdir(PROC_SILEX_LED_TOP, NULL);
	if(parent == NULL) {
		printk(KERN_ERR "ERROR: proc mkdir %s\n", PROC_SILEX_LED_TOP);
		return -1;
	}

	proc_name[7] = '\0';
	for(i=1; i<=LED_NUM; i++, dirp++) {
		snprintf(proc_name, 7, "%d", i);
		led_index[i-1] = i;
		dirp = create_proc_entry(proc_name, 0644, parent);
		if(dirp == NULL) {
			return -1;
		}
		dirp->read_proc = led_proc_read;
		dirp->write_proc = led_proc_write;
		dirp->data = &led_index[i-1];
	}

	sema_init( &led_proc_sem, 1 );

	return 0;
}

/**
 * Cleanup LED proc-fs.
 * 
 */
static void __exit led_proc_exit_module (void)
{
	printk(KERN_INFO "led_proc cleanup nothing done.");
}

module_init(led_proc_init_module);
module_exit(led_proc_exit_module);

