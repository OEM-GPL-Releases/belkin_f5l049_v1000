/**
 * @file
 *
 * ag7100 ethernet link check proc fs
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
#include <linux/delay.h>
#include <asm/sockios.h>
#include <linux/mii.h>
#include <asm/semaphore.h>

#include "ar7100.h"
#include "ag7100_proc.h"

typedef struct {
	int is_enet_port;
	int mac_unit;
	unsigned int phy_addr;
} athr_phy_t;


static athr_phy_t phy_info[] = {
	{ is_enet_port: 1,
	  mac_unit    : 0,
	  phy_addr    : 0x01
	}
};

#define MODULE_NAME             "AG7100_PROCFS"
#define PROC_AG7100_TOP         "silex/ag7100"

static struct semaphore ag7100_proc_sem;

static uint16_t ag7100_proc_mii_read(int unit, uint32_t phy_addr, uint8_t reg)
{
	ar7100_reg_t mac_base = ag7100_mac_base(unit);
	uint16_t addr = (phy_addr << AG7100_ADDR_SHIFT) | reg, val;
	volatile int rddata;
	uint16_t ii = 0x1000;


	ar7100_reg_wr_nf(mac_base + AG7100_MII_MGMT_CMD, 0x0);
	ar7100_reg_wr_nf(mac_base + AG7100_MII_MGMT_ADDRESS, addr);
	ar7100_reg_wr_nf(mac_base + AG7100_MII_MGMT_CMD, AG7100_MGMT_CMD_READ);

	do {
		udelay(5);
		rddata = ar7100_reg_rd(mac_base + AG7100_MII_MGMT_IND) & 0x1;
	} while(rddata && --ii);

	val = ar7100_reg_rd(mac_base + AG7100_MII_MGMT_STATUS);
	ar7100_reg_wr_nf(mac_base + AG7100_MII_MGMT_CMD, 0x0);

	return val;
}

static void ag7100_proc_mii_write(int unit, uint32_t phy_addr, uint8_t reg, uint16_t data)
{
	ar7100_reg_t mac_base = ag7100_mac_base(unit);
	uint16_t      addr  = (phy_addr << AG7100_ADDR_SHIFT) | reg;
	volatile int rddata;
	uint16_t      ii = 0x1000;

	ar7100_reg_wr_nf(mac_base + AG7100_MII_MGMT_ADDRESS, addr);
	ar7100_reg_wr_nf(mac_base + AG7100_MII_MGMT_CTRL, data);

	do {
		rddata = ar7100_reg_rd(mac_base + AG7100_MII_MGMT_IND) & 0x1;
	} while(rddata && --ii);
}

static athr_phy_t* ag7100_proc_phy_find(int unit)
{
	int i;
	athr_phy_t *phy;

	for(i = 0; i < sizeof(phy_info)/sizeof(athr_phy_t); i++) {
		phy = &phy_info[i];
		if (phy->is_enet_port && (phy->mac_unit == unit)) 
			return phy;
	}

	return NULL;
}

static void ag7100_proc_phy_reset(int unit)
{
	uint16_t phy_reg_data;
	volatile uint32_t timeout, i;
	athr_phy_t *phy = ag7100_proc_phy_find(unit);
	if (!phy) 
		return;

	/* Software reset */
	phy_reg_data = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
	ag7100_proc_mii_write(unit, phy->phy_addr, 0, phy_reg_data | 0x8000);

	/* Waiting for reset complete */
	timeout = 1000;
	do {
		if (timeout-- == 0) {
			return;
		}
		for (i = 0; i < 1000; i++);
		phy_reg_data = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
	} while (phy_reg_data & 0x8000);
}

static int ag7100_proc_phy_set_link(int unit, int mode)
{
	uint16_t mode_ctrl = 0;
	uint16_t auto_adv = 0;
	athr_phy_t *phy;

	phy = ag7100_proc_phy_find(unit);
	if (!phy) 
		return -1;

	switch (mode) {
	case AG7100_LINKMODE_AUTO: /* AUTO */
		auto_adv   = ag7100_proc_mii_read(unit, phy->phy_addr, 4);
		auto_adv  |= 0x01e0;
		ag7100_proc_mii_write(unit, phy->phy_addr, 4, auto_adv);
		mode_ctrl = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
		mode_ctrl |= 0x1200;
		ag7100_proc_mii_write(unit, phy->phy_addr, 0, mode_ctrl);
		ag7100_proc_phy_reset(unit);
		break;
	case AG7100_LINKMODE_10HALF: /* Fix 10M HALF */
		mode_ctrl = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
		mode_ctrl &= ~0x3100;
		ag7100_proc_mii_write(unit, phy->phy_addr, 0, mode_ctrl);
		ag7100_proc_phy_reset(unit);
		break;
	case AG7100_LINKMODE_10FULL: /* Fix 10M FULL */
		mode_ctrl = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
		mode_ctrl &= ~0x3000;
		mode_ctrl |= 0x0100;
		ag7100_proc_mii_write(unit, phy->phy_addr, 0, mode_ctrl);
		ag7100_proc_phy_reset(unit);
		break;
	case AG7100_LINKMODE_100HALF: /* Fix 100M HALF */
		mode_ctrl = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
		mode_ctrl &= ~0x1100;
		mode_ctrl |= 0x2000;
		ag7100_proc_mii_write(unit, phy->phy_addr, 0, mode_ctrl);
		ag7100_proc_phy_reset(unit);
		break;
	case AG7100_LINKMODE_100FULL: /* Fix 100M FULL */
		mode_ctrl = ag7100_proc_mii_read(unit, phy->phy_addr, 0);
		mode_ctrl &= ~0x1000;
		mode_ctrl |= 0x2100;
		ag7100_proc_mii_write(unit, phy->phy_addr, 0, mode_ctrl);
		ag7100_proc_phy_reset(unit);
		break;
	default:
		return -1;
	}

	return 0;
}

static int ag7100_proc_phy_is_up(int unit)
{
	int status;
	athr_phy_t *phy = ag7100_proc_phy_find(unit);
	if (!phy) 
		return -1;

	status = ag7100_proc_mii_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	if (status & ATHR_STATUS_LINK_PASS) {
		return 1;
	}

	return 0;
}

#if 0
static int ag7100_proc_phy_is_fdx(int unit)
{
	uint16_t mode_stat;
	int timeout = 0;
	athr_phy_t *phy;

	phy = ag7100_proc_phy_find(unit);
	if (!phy) 
		return -1;

	mode_stat = ag7100_proc_mii_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);

	/* Wating SpeedDuplexResolved */
	while ((mode_stat & 0x0800) == 0) {
		mdelay(10); /* 10msec */
		mode_stat = ag7100_proc_mii_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
		timeout++;
		if (timeout > 500) break;
	}

	/* Half/Full check */
	if ((mode_stat & 0x2000) == 0x2000) {
		return 1;	/* Full Duplex */
	}
	else {
		return 0;	/* Half Duplex */
	}
}

static int ag7100_proc_phy_get_speed(int unit)
{
	uint16_t mode_stat;
	int timeout = 0;
	athr_phy_t *phy;

	phy = ag7100_proc_phy_find(unit);
	if (!phy) 
		return -1;

	mode_stat = ag7100_proc_mii_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);

	/* Wating SpeedDuplexResolved */
	while ((mode_stat & 0x0800) == 0) {
		mdelay(10); /* 10msec */
		mode_stat = ag7100_proc_mii_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
		timeout++;
		if (timeout > 500) break;
	}

	/* 100/10 check */
	if ((mode_stat & 0xc000) == 0x4000) {
		return AG7100_PHY_SPEED_100TX;
	}
	else {
		return AG7100_PHY_SPEED_10T;
	}

	return -1;
}
#endif /* 0 */

/** 
 * Read call back function from proc-fs.
 * 
 * @return Created data length.
 * @return <0 error
 */
static int ag7100_proc_read(char *buf, char **start, off_t offset,
										int count, int *eof, void *data)
{
	int len = 0;

	if ( down_interruptible( &ag7100_proc_sem ) ) {
		printk( KERN_INFO "%s:down_interruptible failed\n", MODULE_NAME);
		return -ERESTARTSYS;
	}

	len += sprintf(buf, "%d\n", ag7100_proc_phy_is_up(0));
	up( &ag7100_proc_sem );

	*eof = 1;
	return len;
}

#define MAX_BUF_SIZE 64

/** 
 * Write call back function from proc-fs.
 * 
 * @return Handled data length.
 * @return <0 error
 */
static int ag7100_proc_write(struct file* filp, const char* buf, 
											unsigned long count, void* data)
{
	int ret;
	char tmpbuf[MAX_BUF_SIZE];
	int mode;

	if(count > MAX_BUF_SIZE)
		return -EINVAL;

	if ( down_interruptible( &ag7100_proc_sem ) ) {
		printk( KERN_INFO "%s:down_interruptible failed\n", MODULE_NAME );
		return -EFAULT;
	}

	if ( copy_from_user( tmpbuf, buf, count) ) {
		up( &ag7100_proc_sem );
		printk( KERN_INFO "%s:copy_from_user failed. count %ld\n",
													MODULE_NAME, count);
		return -EFAULT;
	}
	tmpbuf[MAX_BUF_SIZE -1] = '\0';

	/* Interval time */
	ret = sscanf(tmpbuf, "%d", &mode);

	if(mode < AG7100_LINKMODE_AUTO || mode > AG7100_LINKMODE_MAX) {
		up( &ag7100_proc_sem );
		printk(KERN_INFO "%s:Invalid mode %d\n", MODULE_NAME, mode);
		return -EINVAL;
	}

	ag7100_proc_phy_set_link(0, mode);
	up( &ag7100_proc_sem );

	return count;

}

static int __init ag7100_proc_init(void)
{
	int unit = 0;
	uint16_t func_ctrl = 0;
	uint16_t phy_reg_data = 0;
	uint32_t dw;
	athr_phy_t *phy = ag7100_proc_phy_find(unit);
	ar7100_reg_t mac_base = ag7100_mac_base(unit);

	struct proc_dir_entry *parent;
	struct proc_dir_entry *dirp;
	char proc_name[2];
	proc_name[1] = '\0';

	if (!phy) {
		printk(KERN_ERR "%s:Not found Ethernet PHY\n", MODULE_NAME);
		return -1;
	}

	dw = ar7100_reg_rd(AR7100_RESET);
	/* ETH0 MAC and PHY Reset */
	ar7100_reg_wr_nf(AR7100_RESET, dw | 0x00000300);
	mdelay(100); /* 100msec */

	/* Reset clear */
	ar7100_reg_wr_nf(AR7100_RESET, dw & ~0x00000300);
	mdelay(100); /* 100msec */

	ar7100_reg_wr_nf(mac_base + AG7100_MAC_CFG1, 0);
	mdelay(100); /* 100msec */

	/* MII0 Control */
	ar7100_reg_wr_nf(AR7100_MII0_CTRL, 0x01); /* MII:0x1, RMII:0x3, RGMII:0x2 */

	/* Ethernet Clock Control for EtherPHY */
	ar7100_reg_wr_nf(AR9100_ETH_EXT_CLK, 0x0909);

	/* MII Configration (Source clock divided by 20) */
	ar7100_reg_wr_nf(mac_base + AG7100_MAC_MII_MGMT_CFG,
	                                         AG7100_MGMT_CFG_CLK_DIV_20);

	/* disable Auto down speed: PHY register 0x14 bit 5 set to 0(ResetUpdate) */
	phy_reg_data = ag7100_proc_mii_read(unit, phy->phy_addr, 0x14);
	ag7100_proc_mii_write(unit, phy->phy_addr, 0x14, phy_reg_data & ~0x20);

	/* Auto MDI/MDIX Enable */
	func_ctrl =  ag7100_proc_mii_read(unit, phy->phy_addr, 16);
	func_ctrl |= 0x0060;
	ag7100_proc_mii_write(unit, phy->phy_addr, 16, func_ctrl);

	/* PHY Reset */
	ag7100_proc_phy_reset(unit);

	/* Init PROCFS */
	parent = proc_mkdir(PROC_AG7100_TOP, NULL);
	if(parent == NULL) {
		printk(KERN_ERR "ERROR: proc mkdir %s\n", PROC_AG7100_TOP);
		return -1;
	}

	dirp = create_proc_entry("link_status", 0644, parent);
	if(dirp == NULL) {
		printk(KERN_ERR "%s:create_proc_entry failed\n", MODULE_NAME);
		return -1;
	}
	dirp->read_proc = ag7100_proc_read;
	dirp->write_proc = ag7100_proc_write;
	dirp->data = NULL;

	sema_init( &ag7100_proc_sem, 1 );

	return 0;
}

module_init(ag7100_proc_init);
