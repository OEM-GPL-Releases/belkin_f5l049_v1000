/**
 * @file
 *
 * Switch control proc fs
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
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
#include <linux/syscalls.h>

#include "cfg_sw.h"
#include "sw_ctrl.h"

#define PROC_SILEX_SW_TOP	"silex/sw"
#define MAX_BUF_SIZE 256
#define MODULE_NAME "SXSW_PROC"

static struct proc_dir_entry *sw_dirp[SW_NUM];
static int sw_index[SW_NUM];

struct sx_sw_prog {
	struct proc_dir_entry *dirp;
	char prog[MAX_BUF_SIZE];
	struct semaphore sem;
};

static struct sx_sw_prog sw_prog;

/** 
 * Read proc-fs callback func.
 * 
 * @return len data length.
 */
static int sw_proc_read(char *buf, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;
	int *index = (int*)data;

	len += sprintf(buf, "%d\n", sx_sw_read(*index));

	*eof = 1;
	return len;
}

static void sw_proc_event(int sw_num, int action, unsigned long msec)
{
	int ret;
	char *argv_init[2];
	char *envp_init[8];
	char env_action[16];
	char env_switch[16];
	char env_time[16];

	if ( down_interruptible( &sw_prog.sem ) ) {
		printk( KERN_INFO "%s:down_interruptible failed\n", MODULE_NAME);
		return;
	}

	/* Command argument */
	argv_init[0] = sw_prog.prog;
	argv_init[1] = NULL;

	/* Command environment */
	snprintf(env_switch, 15, "SW=%d", sw_num);
	snprintf(env_action, 15, "ACTION=%d", action);
	snprintf(env_time, 15, "TIME=%ld", msec);

	envp_init[0] = env_switch;
	envp_init[1] = env_action;
	envp_init[2] = env_time;
	envp_init[3] = NULL;

	ret = call_usermodehelper(sw_prog.prog, argv_init, envp_init, UMH_WAIT_PROC);
	if(ret < 0) {
		printk(KERN_ERR "%s:%s exec fail %d\n", MODULE_NAME, sw_prog.prog, ret);
	}

	up( &sw_prog.sem );
}

static int sw_proc_read_prog(char *buf, char **start, off_t offset,
											int count, int *eof, void *data)
{
	struct sx_sw_prog *sw_progp = (struct sx_sw_prog*) data;
	int len = 0;

	if ( down_interruptible( &sw_progp->sem ) ) {
		printk( KERN_INFO "%s:down_interruptible failed\n", MODULE_NAME);
		return -EFAULT;
	}

	len = strlen(sw_progp->prog);
	strncpy(buf, sw_progp->prog, len);
	*eof = 1;

	up( &sw_progp->sem );

	return len;
}

static int sw_proc_write_prog(struct file* filp, const char* buf, 
											unsigned long count, void* data)
{
	struct sx_sw_prog *sw_progp = (struct sx_sw_prog*) data;
	int ret;

	if(count > MAX_BUF_SIZE)
		return -EINVAL;

	if ( down_interruptible( &sw_progp->sem ) ) {
		printk( KERN_ERR "%s:down_interruptible failed\n", MODULE_NAME);
		return -EFAULT;
	}

	if ( copy_from_user( sw_progp->prog, buf, count ) ) {
		up( &sw_progp->sem );
		printk( KERN_ERR "%s:copy_from_user failed\n", MODULE_NAME);
		return -EFAULT;
	}
	sw_progp->prog[count] = '\0';
	
	ret = sx_sw_set_event(sw_proc_event);
	if(ret < 0) {
		printk(KERN_ERR "%s:sx_sw_set_event fail\n", MODULE_NAME);
		up( &sw_progp->sem );
		return ret;
	}

	up( &sw_progp->sem );

	return count;
}

/** 
 * Initial Switch proc-fs.
 * 
 * @return 0 success
 * @return <0 error
 */
static int __init sw_proc_init_module (void)
{
	int i;
	char proc_name[2];
	struct proc_dir_entry *parent;
	struct proc_dir_entry *dirp = sw_dirp[0];

	parent = proc_mkdir(PROC_SILEX_SW_TOP, NULL);
	if(parent == NULL) {
		printk(KERN_ERR "%s:proc mkdir %s\n", MODULE_NAME, PROC_SILEX_SW_TOP);
		return -1;
	}

	/* proc fs init for getting push switch status */
	proc_name[1] = '\0';
	for(i=1; i<=SW_NUM; i++, dirp++) {
		proc_name[0] = i + '0';
		sw_index[i-1] = i;
		dirp = create_proc_entry(proc_name, 0644, parent);
		if(dirp == NULL) {
			printk(KERN_ERR "%s:Error create_proc_entry() proc name %s\n",
														MODULE_NAME, proc_name);
			return -1;
		}
		dirp->read_proc = sw_proc_read;
		dirp->data = &sw_index[i-1];
	}

	/* proc fs init for Setting event program */
	sw_prog.dirp = create_proc_entry("sw_prog", 0644, parent);
	if(sw_prog.dirp == NULL) {
		printk(KERN_ERR "%s:Error create_proc_entry() sw_prog\n", MODULE_NAME);
		return -1;
	}

	sw_prog.dirp->write_proc = sw_proc_write_prog;
	sw_prog.dirp->read_proc = sw_proc_read_prog;
	sw_prog.dirp->data =(void*)&sw_prog;
	sema_init(&sw_prog.sem, 1);
	sw_prog.prog[0] = '\0';

	return 0;
}

/** 
 * Cleanup Switch proc-fs.
 * 
 */
static void __exit sw_proc_exit_module (void)
{
	printk(KERN_INFO "%s:cleanup nothing done.\n", MODULE_NAME);
}

module_init(sw_proc_init_module);
module_exit(sw_proc_exit_module);
