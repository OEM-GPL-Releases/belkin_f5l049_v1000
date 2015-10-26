/**
 * @file
 *
 * silex standard Flash ROM Interface
 * 
 * Copyright (C) 2008 - 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/fs.h>
#include <asm/byteorder.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "sxflash.h"

#ifdef CONFIG_ARCH_SX5500
#include "s29jl032h.h"
#endif

#ifdef CONFIG_MACH_AR7100
#include "mx25l.h"
#endif

#define MODULE_NAME "SXFLASH"
#define SXFLASH_VERSION "1.0.0"

static struct sectinfo sect_eeprom;			/* Setting sector */
static struct sectinfo sect_app;			/* Application sector */
static struct sectinfo sect_rootfs;			/* rootfs sector */

static struct mtd_partition mtdp_rootfs;
static struct mtd_info mtdi_rootfs;

#ifdef CONFIG_SXFLASH_ART
static struct sectinfo sect_art;			/* art sector */
#endif

static struct sxflash_operations flashop;	/* standard read write erase function pointer */
static F_BLOCK **ep_current, **ep_backup;	/* eeprom management tabble */
int sxflash_eeprom_need_factory_default = 0;

#define _DEBUG_ 0

static int sxflash_init_sect(struct sectinfo *sectp, F_BLOCK **btablep, 
								struct sxflash_operations *flashop, 
								unsigned long flags,
								const char *sectname)
{
	int size = 0;
	F_BLOCK **bp = btablep;

	if(btablep == NULL || *btablep == F_END_FROM)
		return -1;

	strncpy(sectp->name, sectname, 31);

	while((*bp) != F_END_FROM) {
#if _DEBUG_
		printk(MODULE_NAME "ADDR: %x SIZE %dByte\n", (*bp)->offset, (*bp)->size);
#endif /* _DEBUG_ */
		size += (*bp)->size;
		bp++;
	}

	sectp->btablep = btablep;
	sectp->size = size;
	sectp->func = flashop;
	sectp->flags = flags;
	sectp->open_cnt = 0;
	sectp->write_open_cnt = 0;
	init_MUTEX(&sectp->lock);
	printk(KERN_INFO MODULE_NAME ":%s bind size %dByte\n", sectp->name, sectp->size);

	return 0;
}

static void sxflash_release_cache(struct sectinfo *sectp)
{
	int i;

	if(sectp->cachep != NULL) {
		for(i = 0; i < sectp->caches; i++) {
			if(sectp->cachep[i] != NULL) {
				kfree(sectp->cachep[i]);
			}
		}
		kfree(sectp->cachep);
		sectp->cachep = NULL;
	}

	sectp->caches = 0;
}

static int sxflash_create_cache(struct sectinfo *sectp)
{
	unsigned char **cachep;
	int size = sectp->size;
	int i;
	int num;
	int oddlen;

	if(size == 0)
		return -1;

	num = size / SXFLASH_CACHE_SIZE;
	oddlen = size % SXFLASH_CACHE_SIZE;

	sectp->caches = num;
	if(oddlen != 0) {
		sectp->caches++;
	}

#if _DEBUG_
printk(MODULE_NAME "create: num %d oddlen %d\n", num, oddlen);
printk(MODULE_NAME "create: caches %d\n", sectp->caches);
#endif /* _DEBUG_ */

	cachep = (unsigned char**)kmalloc(sizeof(unsigned char*) * (sectp->caches), 
															GFP_KERNEL);
	if(cachep == NULL)
		return -ENOMEM;

	for(i=0; i<num; i++) {
		cachep[i] = (unsigned char*)kmalloc(SXFLASH_CACHE_SIZE, GFP_KERNEL);
		if(cachep[i] == NULL) {
			sxflash_release_cache(sectp);
			return -ENOMEM;
		}
#if _DEBUG_
printk(MODULE_NAME "create: cache[i] %p\n", cachep[i]);
#endif /* _DEBUG_ */
	}
	if(oddlen != 0) {
		cachep[num] = (unsigned char*)kmalloc(oddlen, GFP_KERNEL);
		if(cachep[num] == NULL) {
			sxflash_release_cache(sectp);
			return -ENOMEM;
		}
#if _DEBUG_
printk(MODULE_NAME "create: oddlen cache[i] %p\n", cachep[num]);
#endif /* _DEBUG_ */
	}

	sectp->cachep = cachep;

	return 0;
}

static void sxflash_reverse_cache(struct sectinfo *sectp)
{
	int i, j, rlen, len;
	unsigned char *bufp;

	rlen = sectp->size;
	for(i=0; i<sectp->caches; i++) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;
		
		bufp = sectp->cachep[i];
		for(j=0; j<len; j++)
			bufp[j] = ~bufp[j];

		rlen -= len;
	}
}

static void sxflash_memset_cache(struct sectinfo *sectp, u8 filldata)
{
	int i, rlen, len;

	rlen = sectp->size;
	for(i=0; i<sectp->caches; i++) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;

		memset(sectp->cachep[i], filldata, len);

		rlen -= len;
	}
}

static int sxflash_load_cache(struct sectinfo *sectp)
{
	int ret, i, offset = 0;
	int rlen, len;

	if(sectp->cachep == NULL || sectp->func == NULL)
		return -1;

#if _DEBUG_
printk(MODULE_NAME ":%s load cache start\n", sectp->name);
#endif /* _DEBUG_ */

	rlen = sectp->size;
	for(i=0; i<sectp->caches; i++) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;

		ret = sectp->func->read(sectp->btablep, offset, len, sectp->cachep[i]);
		if(ret < 0)
			return ret;

		offset += len;
		rlen -= len;
	}

	if((sectp->flags & SXFLASH_SECT_REVERSAL) != 0)
		sxflash_reverse_cache(sectp);

	return 0;
}

static int sxflash_save_cache(struct sectinfo *sectp)
{
	int ret = 0, i, offset = 0;
	int wlen, len;

	if(sectp->cachep == NULL || sectp->func == NULL)
		return -1;

	ret = sectp->func->erase_sect(sectp->btablep);
	if(ret < 0)
		return ret;

	if((sectp->flags & SXFLASH_SECT_REVERSAL) != 0)
		sxflash_reverse_cache(sectp);

	wlen = sectp->size;
	for(i=0; i<sectp->caches; i++) {
		if(wlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = wlen;

		ret = sectp->func->write(sectp->btablep, offset, len, sectp->cachep[i]);
		if(ret < 0)
			break;

		offset += len;
		wlen -= len;
	}

	if((sectp->flags & SXFLASH_SECT_REVERSAL) != 0)
		sxflash_reverse_cache(sectp);

	if(ret >= 0) {
		sectp->flags &= ~SXFLASH_SECT_OVERWRITED;  /* Already saved flash */
		ret = 0;   /* Success */
	}

	return ret;
}

static int sxflash_check_cache(struct sectinfo *sectp)
{
	int i, rlen, len;
	int sum = 0;
	int cnt;
	u16 *wdp;

	rlen = sectp->size;
	for(i=0; i<sectp->caches; i++) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;

		wdp = (u16*)sectp->cachep[i];
		for (cnt = 0; cnt < len; cnt += sizeof(u16))
			sum += *wdp++;

		rlen -= len;
	}

	if((sum & 0xffff) == 0xffff)
		return 0;
	else
		return -1;
}

static int sxflash_read_cache(struct sectinfo *sectp, unsigned long offset, int size, unsigned char *buf)
{
	unsigned char **cachep = sectp->cachep;
	int cachei;
	unsigned long coffset;					/* offset in cache */
	unsigned int rlen;
	unsigned int len;
	unsigned char *bufp = buf;

	if(cachep == NULL) {
		printk(KERN_ERR MODULE_NAME ":%s area is not initialized\n", sectp->name);
		return -1;
	}

	if (size == 0)
		return 0;

	if(offset + size > sectp->size)
		size = sectp->size - offset;

	if(size == 0)
		return -ENOSPC;

	rlen = size;

	cachei = offset / SXFLASH_CACHE_SIZE;
	coffset = offset % SXFLASH_CACHE_SIZE;

	if(coffset != 0) {
		if(rlen > SXFLASH_CACHE_SIZE - coffset)
			len = SXFLASH_CACHE_SIZE - coffset;
		else
			len = rlen;

		memcpy(bufp, cachep[cachei] + coffset, len);

		cachei++;
		rlen -= len;
		bufp += len;
	}

	while(rlen > 0) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;

		memcpy(bufp, cachep[cachei], len);

		cachei++;
		rlen -= len;
		bufp += len;
	}

	return size;
}

static ssize_t sxflash_write_cache(struct sectinfo *sectp, unsigned long offset, int size, unsigned char *buf)
{
	unsigned char **cachep = sectp->cachep;
	int cachei;
	unsigned long coffset;					/* offset in cache */
	unsigned int wlen;
	unsigned int len;

	if(cachep == NULL) {
		printk(KERN_ERR MODULE_NAME ":%s area is not initialized\n", sectp->name);
		return -1;
	}

	if(size == 0)
		return 0;

	if(offset + size > sectp->size)
		size = sectp->size - offset;
	
	if(size == 0)
		return -ENOSPC;

	wlen = size;

	cachei = offset / SXFLASH_CACHE_SIZE;
	coffset = offset % SXFLASH_CACHE_SIZE;

	if(coffset != 0) {
		if(wlen > SXFLASH_CACHE_SIZE - coffset)
			len = SXFLASH_CACHE_SIZE - coffset;
		else
			len = wlen;

		memcpy(cachep[cachei] + coffset, buf, len);

		cachei++;
		wlen -= len;
	}

	while(wlen > 0) {
		if(wlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = wlen;
		
		memcpy(cachep[cachei], buf, len);

		cachei++;
		wlen -= len;
	}

	return size;
}

static u16 sxflash_calc_checksum(unsigned char *p, int len)
{
	u16 sum;

	sum = 0;
	while (len-- > 0) sum += *p++;

	return (sum);
}

static u16 sxflash_calc_eeprom_checksum(unsigned long offset, int len)
{
	u16 sum;
	u8 tmp;

	sum = SXFLASH_EEP_SUMINIT;
	while (len > 0) {
		tmp = sxflash_epread8(offset);

		offset++;
		len--;
		sum += tmp;
	}

	return (sum);
}

static int sxflash_eeprom_flushcache(void)
{
	struct sectinfo *sectp = &sect_eeprom;
	F_BLOCK **ep;
	u16 calc_sum;
	u32 wcnt;
	int ret;

	if((sectp->flags & SXFLASH_SECT_OVERWRITED) == 0) {
		printk(KERN_INFO MODULE_NAME ":No change cache data\n");
		return 0;
	}

	sxflash_eplock();
	wcnt = sxflash_epread32(SXFLASH_WRITE_CNT_OFFSET);
	wcnt++;
	sxflash_epwrite32(SXFLASH_WRITE_CNT_OFFSET, wcnt);

	/* Modify System area checksum */
	sxflash_epwrite16(SXFLASH_SYS_CHECKSUM_OFFSET, 0x0000);
	calc_sum = sxflash_calc_eeprom_checksum(SXFLASH_SYSTEM, SXFLASH_SYS_SIZE);
	sxflash_epwrite16(SXFLASH_SYS_CHECKSUM_OFFSET, calc_sum);

	/* Modify config area checksum */
	sxflash_epwrite16(SXFLASH_CHECKSUM_OFFSET, 0x0000);
	calc_sum = sxflash_calc_eeprom_checksum(SXFLASH_CONFIG, sectp->size - SXFLASH_SYS_SIZE);
	sxflash_epwrite16(SXFLASH_CHECKSUM_OFFSET, calc_sum);

	ret = sxflash_save_cache(sectp);
	if(ret < 0) {
		sxflash_epunlock();
		printk(KERN_ERR MODULE_NAME ":Error Save cache %d\n", ret);
		return ret;
	}
	else
		sectp->flags &= ~SXFLASH_SECT_OVERWRITED;

	/* Exchange current and backup */
	ep = ep_current;
	ep_current = ep_backup;
	ep_backup = ep;

	/* Stupid mirroring specification */
	/* Write sector is backup sector */
	sectp->btablep = ep_backup;
	sxflash_epunlock();

	printk(KERN_WARNING MODULE_NAME ":%s write success\n", sectp->name);

	return 0;
}

static int sxflash_eeprom_setdefault(void)
{
	struct sectinfo *sectp = &sect_eeprom;
	u32 wcnt;
	u32 setdefcnt;
	unsigned long offset;

	sxflash_eplock();

	/* Backup write count */
	wcnt = sxflash_epread32(SXFLASH_WRITE_CNT_OFFSET);
	setdefcnt = sxflash_epread32(SXFLASH_SETDEF_CNT_OFFSET);

	for(offset = SXFLASH_CONFIG; offset < sectp->size - SXFLASH_SYS_SIZE; offset++) {
		sxflash_epwrite8(offset, 0);
	}

	/* Write back write count */
	sxflash_epwrite32(SXFLASH_WRITE_CNT_OFFSET, wcnt);
	sxflash_epwrite32(SXFLASH_SETDEF_CNT_OFFSET, setdefcnt + 1);

	sectp->flags |= SXFLASH_SECT_OVERWRITED;
	sxflash_epunlock();

	printk(KERN_WARNING MODULE_NAME ":%s setdefault success\n", sectp->name);

	return 0;
}

static int sxflash_verify_eeprom_area(void)
{
	struct sectinfo *sectp = &sect_eeprom;
	unsigned char etheraddr[6];
	u16 sum;
	u16 calc_sum;

	if(sectp == NULL || sectp->func == NULL)
		return -1;

	sxflash_eplock();

	/* Check Ethernet Address Checksum */
	sum = sxflash_epread16(SXFLASH_ETHER_SUM_OFFSET);
	sxflash_epread(SXFLASH_ETHER_ADDR_OFFSET, 6, (unsigned char*)etheraddr);
	calc_sum = sxflash_calc_checksum(etheraddr, 6);
	if(sum != calc_sum) {
		printk(KERN_ERR MODULE_NAME ":verify_eeprom:sum error1 %x %x\n", sum, calc_sum);
		sxflash_epunlock();
		return -1;
	}

	/* Check System area Checksum */
	sum = sxflash_epread16(SXFLASH_SYS_CHECKSUM_OFFSET);
	sxflash_epwrite16(SXFLASH_SYS_CHECKSUM_OFFSET, 0);
	calc_sum = sxflash_calc_eeprom_checksum(SXFLASH_SYSTEM, SXFLASH_SYS_SIZE);
	sxflash_epwrite16(SXFLASH_SYS_CHECKSUM_OFFSET, calc_sum);
	if(sum != calc_sum) {
		sxflash_epunlock();
		printk(KERN_ERR MODULE_NAME ":verify_eeprom:sum error2 %x %x\n", sum, calc_sum);
		return -1;
	}

	sum = sxflash_epread16(SXFLASH_CHECKSUM_OFFSET);
	sxflash_epwrite16(SXFLASH_CHECKSUM_OFFSET, 0);
	calc_sum = sxflash_calc_eeprom_checksum(SXFLASH_CONFIG, sectp->size - SXFLASH_SYS_SIZE);
	sxflash_epwrite16(SXFLASH_CHECKSUM_OFFSET, calc_sum);
	if(sum != calc_sum) {
		sxflash_epunlock();
		printk(KERN_ERR MODULE_NAME ":verify_eeprom:sum error3 %x %x\n", sum, calc_sum);
		return -1;
	}

	sxflash_epunlock();

	return 0;
}

/*****************************************************************************
 *                                                                           *
 * Setting area file oprations.                                              *
 *                                                                           *
 *****************************************************************************/
static int sxflash_open_eeprom(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct sectinfo *sectp = &sect_eeprom;

	if (minor < 0)
		return -ENODEV;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	/* Check if someone writes the flash */
	if(sectp->write_open_cnt != 0) {
		up(&sectp->lock);
		return -EBUSY;
	}

	if (file->f_mode & FMODE_WRITE) {
		/* Someone open the driver */
		if(sectp->open_cnt != 0){
			up(&sectp->lock);
			return -EBUSY;
		}
		sectp->write_open_cnt = 1;
	}

	sectp->open_cnt++;
	file->private_data = &sect_eeprom;

	up(&sectp->lock);

	return 0;
}

static int sxflash_release_eeprom(struct inode *inode, struct file *file)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	if (file->f_mode & FMODE_WRITE)
		sectp->write_open_cnt = 0;

	sectp->open_cnt--;

	/* Anyway! */
	file->private_data = NULL;

	up(&sectp->lock);

	return 0;
}

/*
 * Flush the buffer into EEPROM.
 * This can write the buffer into another EEPROM sector.
 */
static int sxflash_ioctl_eeprom(struct inode *inodep, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret;
/*	struct sectinfo *sectp = (struct sectinfo*)filep->private_data; */
	unsigned char buf[32];	/* buffer for copy_to_user, copy_from_user */
							/* If more buffer is needed, plase change buffer size */

	memset(buf, 0, 32);

	switch (cmd) {
	case SXFLASH_IOCS_FLUSHCACHE:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = sxflash_eeprom_flushcache();
		break;

	case SXFLASH_IOCS_SETDEFAULT:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = sxflash_eeprom_setdefault();
		break;

	case SXFLASH_IOCS_ETHEADDR:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(buf, (unsigned char*)arg, 6);
		if(ret < 0)
			goto ERROR;

		ret = sxflash_set_etheraddr(buf);
		break;

	case SXFLASH_IOCG_ETHEADDR:
		ret = sxflash_get_etheraddr(buf);
		if(ret < 0)
			goto ERROR;

		ret = copy_to_user((unsigned char*)arg, buf, 6);
		if(ret > 0)
			ret = 0; /* Success copy_to_user */

		break;

	case SXFLASH_IOCS_SERIES_NAME:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(buf, (unsigned char*)arg, 15);
		if(ret < 0)
			goto ERROR;

		ret = sxflash_set_seriesname(buf);
		break;

	case SXFLASH_IOCG_SERIES_NAME:
		ret = sxflash_get_seriesname(buf);
		if(ret < 0)
			goto ERROR;

		ret = copy_to_user((unsigned char*)arg, buf, 15);
		if(ret > 0)
			ret = 0; /* Success copy_to_user */

		break;

	case SXFLASH_IOCS_MACHINE_TYPE:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(buf, (unsigned char*)arg, 15);
		if(ret < 0)
			goto ERROR;
		
		ret = sxflash_set_machinetype(buf);
		break;

	case SXFLASH_IOCG_MACHINE_TYPE:
		ret = sxflash_get_machinetype(buf);
		if(ret < 0)
			goto ERROR;

		ret = copy_to_user((unsigned char*)arg, buf, 15);
		if(ret > 0)
			ret = 0; /* Success copy_to_user */

		break;

	case SXFLASH_IOCS_VERSION:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(buf, (unsigned char*)arg, 15);
		if(ret < 0)
			goto ERROR;
		
		ret = sxflash_set_version(buf);
		break;

	case SXFLASH_IOCG_VERSION:
		ret = sxflash_get_version(buf);
		if(ret < 0)
			goto ERROR;

		ret = copy_to_user((unsigned char*)arg, buf, 15);
		if(ret > 0)
			ret = 0; /* Success copy_to_user */

		break;
	
	case SXFLASH_IOCG_CONFLEN:
	{
		unsigned long conflen;
		conflen = sxflash_get_conflen();

		ret = copy_to_user((unsigned int*)arg, &conflen, sizeof(unsigned long));
		if(ret > 0)
			ret = 0; /* Success copy_to_user */
	}
		break;

	case SXFLASH_IOCS_DEBUG:
	{
		int enable;

		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}

		ret = copy_from_user(&enable, (int*)arg, sizeof(int));
		if(ret < 0)
			goto ERROR;

		ret = sxflash_set_debugmode(enable);
	}
		break;

	case SXFLASH_IOCG_DEBUG:
	{
		int enable;

		enable = sxflash_get_is_debugmode();
		ret = copy_to_user((int*)arg, &enable, sizeof(int));
		if(ret > 0)
			ret = 0; /* Success copy_to_user */
	}
		break;

	case SXFLASH_IOCS_SERIALNUM:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(buf, (unsigned char*)arg, 31);
		if(ret < 0)
			goto ERROR;
		
		ret = sxflash_set_serialnum(buf);
		break;

	case SXFLASH_IOCG_SERIALNUM:
		ret = sxflash_get_serialnum(buf);
		if(ret < 0)
			goto ERROR;

		ret = copy_to_user((unsigned char*)arg, buf, 31);
		if(ret > 0)
			ret = 0; /* Success copy_to_user */

		break;

	default:
		ret = -EINVAL;
		break;
	}

ERROR:

	return ret;
}

static ssize_t sxflash_read_eeprom(struct file *file,  char __user *buf, size_t size, loff_t *ppos)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;
	unsigned char **cachep = sectp->cachep;
	int cachei;
	unsigned long offset = *ppos;
	unsigned long coffset;					/* offset in cache */
	unsigned int rlen;			/* read len */
	unsigned int len;			/* tmp len */
	unsigned int flen;			/* file len */
	int ret;

	if (size == 0)
		return 0;

#if _DEBUG_
printk(MODULE_NAME "sxflash_read offset 0x%x %d\n", offset, size);
#endif /* _DEBUG_ */

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	/* Get file size */
	flen = sxflash_epread32(SXFLASH_FILELEN_OFFSET);
	if(flen == 0) {
		up(&sectp->lock);
		printk(KERN_INFO MODULE_NAME ":No file exist\n");
		return -ENOSPC;
	}

	if(offset + size > flen)
		size = flen - offset;

	if(size == 0) {
		up(&sectp->lock);
		return 0;
	}
	rlen = size;

	/* Protect system area */
	offset += SXFLASH_SYSTEMCONF_START;

	cachei = offset / SXFLASH_CACHE_SIZE;
	coffset = offset % SXFLASH_CACHE_SIZE;

	if(coffset != 0) {
		if(rlen > SXFLASH_CACHE_SIZE - coffset)
			len = SXFLASH_CACHE_SIZE - coffset;
		else
			len = rlen;
#if _DEBUG_
printk(MODULE_NAME "sxflash_read addr %p len %d\n", cachep[cachei] + coffset, len);
#endif /* _DEBUG_ */

		ret = copy_to_user(buf, cachep[cachei] + coffset, len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		rlen -= len;
		buf += len;
	}

	while(rlen > 0) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;
#if _DEBUG_
printk(MODULE_NAME "sxflash_read addr %p len %d\n", cachep[cachei], len);
#endif /* _DEBUG_ */

		ret = copy_to_user(buf, cachep[cachei], len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		rlen -= len;
		buf += len;
	}

	*ppos += size;

	up(&sectp->lock);

	return size;
}

static ssize_t sxflash_write_eeprom(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;
	unsigned char **cachep = sectp->cachep;
	int cachei;
	unsigned long offset = *ppos;
	unsigned long coffset;					/* offset in cache */
	unsigned int wlen;
	unsigned int len;
	int ret;

	if(size == 0)
		return 0;
	
	if(down_interruptible(&sectp->lock))
		return -EINTR;

#if _DEBUG_
printk(MODULE_NAME "offset %lx %d %d \n", offset, size, sectp->size);
#endif /* _DEBUG_ */

	if((offset + size) > (sectp->size - SXFLASH_SYSTEMCONF_START))
		size = (sectp->size - SXFLASH_SYSTEMCONF_START) - offset;
	
	if(size == 0) {
		up(&sectp->lock);
		return -ENOSPC;
	}

	wlen = size;

	/* Protect sytem area */
	offset += SXFLASH_SYSTEMCONF_START;

	cachei = offset / SXFLASH_CACHE_SIZE;
	coffset = offset % SXFLASH_CACHE_SIZE;

#if _DEBUG_
printk(MODULE_NAME "cachei %d coffset %d\n", cachei, coffset);
#endif /* _DEBUG_ */

	if(coffset != 0) {
		if(wlen > SXFLASH_CACHE_SIZE - coffset)
			len = SXFLASH_CACHE_SIZE - coffset;
		else
			len = wlen;

		ret = copy_from_user(cachep[cachei] + coffset, buf, len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		wlen -= len;
		buf += len;
	}
#if _DEBUG_
printk(MODULE_NAME "cachei %d coffset %d wlen %d \n", cachei, coffset, wlen);
#endif /* _DEBUG_ */

	while(wlen > 0) {
		if(wlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = wlen;

		ret = copy_from_user(cachep[cachei], buf, len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		wlen -= len;
		buf += len;
	}

	*ppos += size;

#if _DEBUG_
printk(MODULE_NAME ":write success size %d\n", size);
#endif /* _DEBUG_ */

	/* Save file length to eeprom */
	sxflash_epwrite32(SXFLASH_FILELEN_OFFSET, *ppos);
	sectp->flags |= SXFLASH_SECT_OVERWRITED;	/* Need to save cache into ROM */

	up(&sectp->lock);

	return size;
}

static loff_t sxflash_llseek_eeprom(struct file *file, loff_t offset, int orig)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;

	/* Check overflow */
	switch (orig) {
	case SEEK_SET:
		if ((offset < 0) || (offset >= (sectp->size - SXFLASH_SYSTEMCONF_START))) {
			return -EINVAL;
		}
		file->f_pos = (loff_t)offset;
		break;

	case SEEK_CUR:
		if ((file->f_pos + offset >= (sectp->size - SXFLASH_SYSTEMCONF_START)) || 
						(file->f_pos + offset < 0)){
			return -EINVAL;
		}
		file->f_pos += (loff_t)offset;
		break;

	case SEEK_END:
		if ((offset > 0) || 
			((-1 * offset) > (sectp->size - SXFLASH_SYSTEMCONF_START))) {
			return -EINVAL;
		}
		file->f_pos = (loff_t)(sectp->size - SXFLASH_SYSTEMCONF_START + offset);
		break;

	default:
		return -EINVAL;
	}

	return file->f_pos;
}


/*****************************************************************************
 *                                                                           *
 * Application area file oprations.                                          *
 *                                                                           *
 *****************************************************************************/
static int sxflash_open_app(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct sectinfo *sectp = &sect_app;
	int ret = 0;

	if (minor < 0)
		return -ENODEV;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	/* Check if someone writes the flash */
	if(((file->f_mode & FMODE_WRITE) != 0) && 
					(sectp->write_open_cnt != 0) &&
					(sectp->open_cnt != 0)) {
		ret = -EBUSY;
		goto ERROR;
	}

	/* Create cache at first open!! */
	if(sectp->cachep == NULL) {
		ret = sxflash_create_cache(sectp);
		if(ret < 0) {
			printk(KERN_ERR MODULE_NAME ":Could not create cache %d\n", ret);
			goto ERROR;
		}

		/* Special Specification */
		/* Application's partition clear all cache data in first open.
		 * This partion can not read ROM data but write frimware.
		 * If you need to read ROM data, then change from sxflash_memset_cache(), 
		 * to sxflash_load_cache(). But take a little time ;(
		 */
		sxflash_memset_cache(sectp, 0xff);
	}

	if(file->f_mode & FMODE_WRITE)
		sectp->write_open_cnt++;

	sectp->open_cnt++;
	file->private_data = sectp;

ERROR:
	up(&sectp->lock);

	return ret;
}

static int sxflash_release_app(struct inode *inode, struct file *file)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	if(file->f_mode & FMODE_WRITE)
		sectp->write_open_cnt--;

	sectp->open_cnt--;

	/* Anyway! */
	file->private_data = NULL;

	up(&sectp->lock);

	return 0;
}

/*
 * Flush the buffer into EEPROM.
 * This can write the buffer into another EEPROM sector.
 */
static int sxflash_ioctl_app(struct inode *inodep, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct sectinfo *sectp = (struct sectinfo*)filep->private_data;


	switch (cmd) {
	case SXFLASH_IOCS_FLUSHCACHE:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		sectp->flags |= SXFLASH_SECT_OVERWRITED;
		printk(KERN_INFO MODULE_NAME ":OVERWRITE Flag on! The cache will flush when rebooting\n");
		break;

	case SXFLASH_IOCS_SETDEFAULT:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		sxflash_memset_cache(sectp, 0xff);

		break;

	default:
		ret = -EINVAL;
		break;
	}

ERROR:
	return ret;
}

static ssize_t sxflash_read_app(struct file *file,  char __user *buf, size_t size, loff_t *ppos)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;
	unsigned char **cachep = sectp->cachep;
	int cachei;
	unsigned long offset = *ppos;
	unsigned long coffset;					/* offset in cache */
	unsigned int rlen;						/* read len */
	unsigned int len;						/* tmp len */
	int ret;

	if (size == 0)
		return 0;

#if _DEBUG_
printk(MODULE_NAME "sxflash_read offset 0x%x %d\n", offset, size);
#endif /* _DEBUG_ */

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	if(offset + size > sectp->size)
		size = sectp->size - offset;

	if(size == 0) {
		up(&sectp->lock);
		return 0;
	}
	rlen = size;

	cachei = offset / SXFLASH_CACHE_SIZE;
	coffset = offset % SXFLASH_CACHE_SIZE;

	if(coffset != 0) {
		if(rlen > SXFLASH_CACHE_SIZE - coffset)
			len = SXFLASH_CACHE_SIZE - coffset;
		else
			len = rlen;
#if _DEBUG_
printk(MODULE_NAME "sxflash_read addr %p len %d\n", cachep[cachei] + coffset, len);
#endif /* _DEBUG_ */

		ret = copy_to_user(buf, cachep[cachei] + coffset, len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		rlen -= len;
		buf += len;
	}

	while(rlen > 0) {
		if(rlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = rlen;
#if _DEBUG_
printk(MODULE_NAME "sxflash_read addr %p len %d\n", cachep[cachei], len);
#endif /* _DEBUG_ */

		ret = copy_to_user(buf, cachep[cachei], len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		rlen -= len;
		buf += len;
	}

	*ppos += size;

	up(&sectp->lock);

	return size;
}

static ssize_t sxflash_write_app(struct file *file, const char __user *buf, size_t size, loff_t * ppos)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;
	unsigned char **cachep = sectp->cachep;
	int cachei;
	unsigned long offset = *ppos;
	unsigned long coffset;					/* offset in cache */
	unsigned int wlen;
	unsigned int len;
	int ret = 0;

	if(size == 0)
		return 0;
	
	if(down_interruptible(&sectp->lock))
		return -EINTR;

#if _DEBUG_
printk(MODULE_NAME "offset %lx %d %d \n", offset, size, sectp->size);
#endif /* _DEBUG_ */

	if((offset + size) > sectp->size)
		size = sectp->size - offset;
	
	if(size == 0) {
		up(&sectp->lock);
		return -ENOSPC;
	}

	wlen = size;

	cachei = offset / SXFLASH_CACHE_SIZE;
	coffset = offset % SXFLASH_CACHE_SIZE;

#if _DEBUG_
printk(MODULE_NAME "cachei %d coffset %d\n", cachei, coffset);
#endif /* _DEBUG_ */

	if(coffset != 0) {
		if(wlen > SXFLASH_CACHE_SIZE - coffset)
			len = SXFLASH_CACHE_SIZE - coffset;
		else
			len = wlen;

		ret = copy_from_user(cachep[cachei] + coffset, buf, len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		wlen -= len;
		buf += len;
	}

#if _DEBUG_
printk(MODULE_NAME "cachei %d coffset %d wlen %d \n", cachei, coffset, wlen);
#endif /* _DEBUG_ */

	while(wlen > 0) {
		if(wlen > SXFLASH_CACHE_SIZE)
			len = SXFLASH_CACHE_SIZE;
		else
			len = wlen;

		ret = copy_from_user(cachep[cachei], buf, len);
		if(ret != 0) {
			up(&sectp->lock);
			return -EFAULT;
		}

		cachei++;
		wlen -= len;
		buf += len;
	}

	*ppos += size;

#if _DEBUG_
printk(MODULE_NAME ":write success size %d\n", size);
#endif /* _DEBUG_ */

#if 0
	/* Special specification
	 * Application partition need to set Overwrite flag by ioctl()
	 */
	sectp->flags |= SXFLASH_SECT_OVERWRITED;	/* Need to save cache into ROM */
#endif

	up(&sectp->lock);

	return size;
}

static loff_t sxflash_llseek_app(struct file *file, loff_t offset, int orig)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;

	/* Check overflow */
	switch (orig) {
	case SEEK_SET:
		if ((offset < 0) || (offset >= sectp->size)) {
			return -EINVAL;
		}
		file->f_pos = (loff_t)offset;
		break;

	case SEEK_CUR:
		if ((file->f_pos + offset >= sectp->size) || 
						(file->f_pos + offset < 0)){
			return -EINVAL;
		}
		file->f_pos += (loff_t)offset;
		break;

	case SEEK_END:
		if ((offset > 0) || 
			((-1 * offset) > sectp->size)) {
			return -EINVAL;
		}
		file->f_pos = (loff_t)(sectp->size + offset);
		break;

	default:
		return -EINVAL;
	}

	return file->f_pos;
}

/*****************************************************************************
 *                                                                           *
 * Rootfs area file oprations.                                               *
 *                                                                           *
 *****************************************************************************/
static int sxflash_read_rootfs(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct sectinfo *sectp = mtd->priv;
	int ret;

	if(sectp == NULL || sectp->func == NULL)
		return -EFAULT;

	ret = sectp->func->read(sectp->btablep, from, len, buf);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":read error %d\n", ret);
		return ret;
	}

	*retlen = ret;
	return 0;
}

/*****************************************************************************
 *                                                                           *
 * ART area file oprations.                                                  *
 *                                                                           *
 *****************************************************************************/
#if CONFIG_SXFLASH_ART
static int sxflash_open_art(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct sectinfo *sectp = &sect_art;
	int ret = 0;

	if (minor < 0)
		return -ENODEV;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	/* Check if someone writes the flash */
	if(((file->f_mode & FMODE_WRITE) != 0) && 
					(sectp->write_open_cnt != 0) &&
					(sectp->open_cnt != 0)) {
		ret = -EBUSY;
		goto ERROR;
	}

	/* Create cache at first open!! */
	if(sectp->cachep == NULL) {
		ret = sxflash_create_cache(sectp);
		if(ret < 0) {
			printk(KERN_ERR MODULE_NAME ":Could not create cache %d\n", ret);
			goto ERROR;
		}

		ret = sxflash_load_cache(sectp);
		if(ret < 0) {
			printk(KERN_ERR MODULE_NAME ":Could not load cache %d\n", ret);
			goto ERROR;
		}
	}

	if(file->f_mode & FMODE_WRITE)
		sectp->write_open_cnt++;

	sectp->open_cnt++;
	file->private_data = sectp;

ERROR:
	up(&sectp->lock);

	return ret;
}

static int sxflash_release_art(struct inode *inode, struct file *file)
{
	struct sectinfo *sectp = (struct sectinfo*)file->private_data;
	int ret = 0;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

#if 0
	/* Flush the cache data to Flash ROM when close the file */
	/* But I think better that the user process execute ioctl() for writing 
	 * intentionally.
	 */
	if(sectp->flags & SXFLASH_SECT_OVERWRITED) {
		ret = sxflash_save_cache(sectp);
		if(ret == 0)
			sectp->flags &= ~SXFLASH_SECT_OVERWRITED;
	}
#endif

	if(file->f_mode & FMODE_WRITE)
		sectp->write_open_cnt--;

	sectp->open_cnt--;

	/* Anyway! */
	file->private_data = NULL;

	up(&sectp->lock);

	return ret;
}

static int sxflash_calcwrite_art_checksum(struct sectinfo *sectp)
{
	int ret;
	unsigned long offset;
	u16 sum;
	u16 artlen;
	u16 tmp;

	ret = sxflash_read_cache(sectp, SXFLASH_ART_LENGTH_OFFSET, 2, (unsigned char*)&artlen);
	if(ret < 0)
		return ret;

	artlen = be16_to_cpu(artlen);

	sum = 0;
	ret = sxflash_write_cache(sectp, SXFLASH_ART_CHECKSUM_OFFSET, 2, (unsigned char*)&sum);
	if(ret < 0)
		return ret;

	offset = SXFLASH_ART_LENGTH_OFFSET;
	while (artlen > 0) {
		ret = sxflash_read_cache(sectp, offset, 2, (unsigned char*)&tmp);
		if(ret < 0)
			return -1;

		offset += 2;
		artlen -= 2;
		sum ^= tmp;
	}
	sum ^= 0xFFFF;

	ret = sxflash_write_cache(sectp, SXFLASH_ART_CHECKSUM_OFFSET, 2, (unsigned char*)&sum);

	return ret;
}

static int sxflash_ioctl_art(struct inode *inodep, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct sectinfo *sectp = (struct sectinfo*)filep->private_data;
	unsigned char buf[32];	/* buffer for copy_to_user, copy_from_user */
							/* If more buffer is needed, plase change buffer size */

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	switch (cmd) {
		case SXFLASH_IOCS_FLUSHCACHE:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}

		ret = sxflash_calcwrite_art_checksum(sectp);
		if(ret < 0)
			goto ERROR;

		/* Set Overwrite Flag */
		sectp->flags |= SXFLASH_SECT_OVERWRITED;

		ret = sxflash_save_cache(sectp);

		break;

	case SXFLASH_IOCS_SETDEFAULT:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		sxflash_memset_cache(sectp, 0xff);
		sectp->flags |= SXFLASH_SECT_OVERWRITED;

		break;

	case SXFLASH_IOCG_ETHEADDR:
		ret = sxflash_read_cache(sectp, SXFLASH_ART_ETHER_ADDR_OFFSET, 6, buf);
		if(ret < 0)
			goto ERROR;

		ret = copy_to_user((unsigned char*)arg, buf, 6);
		break;

	case SXFLASH_IOCS_ETHEADDR:
		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(buf, (unsigned char*)arg, 6);
		if(ret < 0)
			goto ERROR;

		ret = sxflash_write_cache(sectp, SXFLASH_ART_ETHER_ADDR_OFFSET, 6, buf);
		if(ret < 0)
			goto ERROR;

		break;

	case SXFLASH_IOCS_REGDOMAIN:
	{
		u16 regDomain;

		if(!(filep->f_mode & FMODE_WRITE)) {
			ret = -EACCES;
			goto ERROR;
		}
		ret = copy_from_user(&regDomain, (unsigned char*)arg, 2);
		if(ret < 0)
			goto ERROR;

		regDomain = cpu_to_be16(regDomain);

		ret = sxflash_write_cache(sectp, SXFLASH_ART_REGDOMAIN_OFFSET, 2, 
													(unsigned char*)&regDomain);
		if(ret < 0)
			goto ERROR;

		break;
	}

	case SXFLASH_IOCG_REGDOMAIN:
	{
		u16 regDomain;

		ret = sxflash_read_cache(sectp, SXFLASH_ART_REGDOMAIN_OFFSET, 2, 
													(unsigned char*)&regDomain);
		if(ret < 0)
			goto ERROR;

		regDomain = be16_to_cpu(regDomain);

		ret = copy_to_user((unsigned char*)arg, &regDomain, 2);
		break;
	}

	default:
		ret = -EINVAL;
		break;
	}

ERROR:
	up(&sectp->lock);

	return ret;
}

static struct file_operations sxart_fops =
{
	.owner		= THIS_MODULE,
	.llseek		= sxflash_llseek_app,          /* Substitute APP */
	.read		= sxflash_read_app,            /* Substitute APP */
	.write		= sxflash_write_app,           /* Substitute APP */
	.ioctl		= sxflash_ioctl_art,
	.open		= sxflash_open_art,
	.release	= sxflash_release_art,
};

static struct miscdevice sxart_miscdev =
{
	SXFLASH_MISC_DEVNUM_ART,
	"sxart",
	&sxart_fops
};
#endif /* CONFIG_SXFLASH_ART */

static struct file_operations sxeeprom_fops =
{
	.owner		= THIS_MODULE,
	.llseek		= sxflash_llseek_eeprom,
	.read		= sxflash_read_eeprom,
	.write		= sxflash_write_eeprom,
	.ioctl		= sxflash_ioctl_eeprom,
	.open		= sxflash_open_eeprom,
	.release	= sxflash_release_eeprom,
};

static struct miscdevice sxeeprom_miscdev =
{
	SXFLASH_MISC_DEVNUM_EEPROM,
	"sxeeprom",
	&sxeeprom_fops
};

static struct file_operations sxapp_fops =
{
	.owner		= THIS_MODULE,
	.llseek		= sxflash_llseek_app,
	.read		= sxflash_read_app,
	.write		= sxflash_write_app,
	.ioctl		= sxflash_ioctl_app,
	.open		= sxflash_open_app,
	.release	= sxflash_release_app,
};

static struct miscdevice sxapp_miscdev =
{
	SXFLASH_MISC_DEVNUM_APP,
	"sxapp",
	&sxapp_fops
};


static int __init sxflash_init(void)
{
	int i, ret;
	unsigned long wcnt0, wcnt1;
	F_BLOCK **ep;
	unsigned char tmpbuf[256];
	unsigned char series[16];
	unsigned char machine[16];
	unsigned char version[16];

	printk(MODULE_NAME "Flash ROM driver v%s\n", SXFLASH_VERSION);

	ret = flash_drv_init(&flashop);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":fail errno %d\n", ret);
		return -1;
	}

	/* EEPROM(setting) sector initializing */
	ret = flashop.read(sxflash_eeprom0_sect, SXFLASH_WRITE_CNT_OFFSET,
													4, (unsigned char*)&wcnt0);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":fail errno %d\n", ret);
		return -1;
	}

	ret = flashop.read(sxflash_eeprom1_sect, SXFLASH_WRITE_CNT_OFFSET,
													4, (unsigned char*)&wcnt1);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":fail errno %d\n", ret);
		return -1;
	}
	wcnt0 = ~be32_to_cpu(wcnt0);
	wcnt1 = ~be32_to_cpu(wcnt1);

	printk(KERN_INFO MODULE_NAME "EEPROM write count: sect0 %lx sect1 %lx\n", wcnt0, wcnt1);

	if(wcnt0 > wcnt1) {
		ep_current = sxflash_eeprom0_sect;
		ep_backup = sxflash_eeprom1_sect;
	}
	else {
		ep_current = sxflash_eeprom1_sect;
		ep_backup = sxflash_eeprom0_sect;
	}

	/* EEPROM initialize state machine */
	for(i=0; i<2; i++) {
		memset(&sect_eeprom, 0, sizeof(struct sectinfo));
		ret = sxflash_init_sect(&sect_eeprom, ep_current, &flashop, 
			SXFLASH_SECT_READABLE | SXFLASH_SECT_WRITABLE | SXFLASH_SECT_REVERSAL,
									"eeprom");
		if(ret < 0)
			return ret;

		ret = sxflash_create_cache(&sect_eeprom);
		if(ret < 0)
			return ret;

		ret = sxflash_load_cache(&sect_eeprom);
		if(ret < 0)
			return ret;

		ret = sxflash_verify_eeprom_area();
		if(ret < 0 && i == 0) {
			/* First Try */
			/* ep_current is broken, So use backup. */
			printk(KERN_ERR MODULE_NAME ":eeprom sector(%d) is broken, use backup\n",
										 (wcnt0 > wcnt1) ? 0 : 1);
			sxflash_release_cache(&sect_eeprom);
			
			/* Exchange current and backup */
			ep = ep_current;
			ep_current = ep_backup;
			ep_backup = ep;
		}
		else if (ret < 0 && i == 1) {
			/* Both sector is Broken */
			printk(KERN_ERR MODULE_NAME ":!!! Both eeprom sector is broken !!! \n");
			printk(KERN_ERR MODULE_NAME ":!!! Found new flash ROM. Use eeprom1 !!!\n");
		}
		else {
			printk(KERN_INFO MODULE_NAME ":Success eeprom init\n");
			break;
		}
	}
	/* Stupid mirroring eeprom sector specification.
	 * Write sector is backup sector.
	 */
	sect_eeprom.btablep = ep_backup;

	ret = misc_register(&sxeeprom_miscdev);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":ERROR eeprom0 driver registrated fail\n");
		return ret;
	}

	sxflash_get_seriesname(series);
	sxflash_get_machinetype(machine);
	sxflash_get_version(version);

	/* APPLICATION sector initializing */
	ret = sxflash_init_sect(&sect_app, sxflash_app_sect, &flashop,
						SXFLASH_SECT_WRITABLE,
						"app");
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":app sector init fail.\n");
		return ret;
	}
	
	/* Load firmware information from Application area */
	ret = sect_app.func->read(sect_app.btablep,
								SXFLASH_FIRMINFO_OFFSET, SXFLASH_FIRMINFO_SIZE,
								tmpbuf);
	if(ret < 0)
		return ret;

	ret = misc_register(&sxapp_miscdev);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":app driver registrated fail\n");
		return ret;
	}

	/* Check difference Firmware and eeprom and renew */
	if((strncmp(series, &tmpbuf[16], 15) != 0) ||
	   (strncmp(machine, &tmpbuf[32], 15) != 0) ||
	   (strncmp(version, &tmpbuf[48], 15) != 0)) {
		printk(KERN_INFO MODULE_NAME ":eeprom Renew firemware info\n");

		printk(KERN_DEBUG MODULE_NAME ":eeprom Series name \"%s\"\n", series);
		printk(KERN_DEBUG MODULE_NAME ":app Series name \"%s\"\n", &tmpbuf[16]);
		printk(KERN_DEBUG MODULE_NAME ":eeprom Machine type \"%s\"\n", machine);
		printk(KERN_DEBUG MODULE_NAME ":app Machine type \"%s\"\n", &tmpbuf[32]);
		printk(KERN_DEBUG MODULE_NAME ":eeprom Version \"%s\"\n", version);
		printk(KERN_DEBUG MODULE_NAME ":app Version \"%s\"\n", &tmpbuf[48]);

		sxflash_set_seriesname(&tmpbuf[16]);
		sxflash_set_machinetype(&tmpbuf[32]);
		sxflash_set_version(&tmpbuf[48]);
		sxflash_set_epoverwrited();
	}

	if(sxflash_eeprom_need_factory_default != 0) {
		ret = sxflash_eeprom_setdefault();
		if(ret < 0) {
			printk(KERN_ERR MODULE_NAME ":eeprom setdefault error %d\n", ret);
			return ret;
		}
		else {
			sxflash_set_epoverwrited();
		}
		printk(KERN_INFO MODULE_NAME ":eeprom set default!\n");
	}

	if((sect_eeprom.flags & SXFLASH_SECT_OVERWRITED) != 0) {
		ret = sxflash_eeprom_flushcache();
		if(ret < 0) {
			printk(KERN_ERR MODULE_NAME "eeprom:flush cache error %d\n", ret);
			return ret;
		}
	}

	/* rootfs sector initializing */
	ret = sxflash_init_sect(&sect_rootfs, sxflash_rootfs_sect, &flashop, 
						SXFLASH_SECT_READABLE,
						"rootfs");
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":rootfs sector init fail.\n");
		return ret;
	}
	memset(&mtdp_rootfs, 0, sizeof(struct mtd_partition));
	memset(&mtdi_rootfs, 0, sizeof(struct mtd_info));

	/* Disk Information init */
	mtdi_rootfs.name = "sxflash";
	mtdi_rootfs.type = MTD_NORFLASH;
	mtdi_rootfs.flags = MTD_CAP_ROM;
//	mtdi_rootfs.ecctype = MTD_ECC_NONE; /* undefined linux kernel 2.6.25.17 */
	mtdi_rootfs.size = sect_rootfs.size + 1;
	mtdi_rootfs.erasesize = 0;
	mtdi_rootfs.writesize = 512;
	mtdi_rootfs.oobsize = 0;
	mtdi_rootfs.owner = THIS_MODULE;

	mtdi_rootfs.erase = NULL;
	mtdi_rootfs.point = NULL;
	mtdi_rootfs.unpoint = NULL;
	mtdi_rootfs.read = sxflash_read_rootfs;
	mtdi_rootfs.write = NULL;
	mtdi_rootfs.read_oob = NULL;
	mtdi_rootfs.write_oob = NULL;
	mtdi_rootfs.sync = NULL;
	mtdi_rootfs.priv = (void*)&sect_rootfs;
	
	/* Partition Information init */
	mtdp_rootfs.name = "rootfs";
	mtdp_rootfs.size = sect_rootfs.size;
	mtdp_rootfs.offset = 0x00000000;

	ret = add_mtd_partitions(&mtdi_rootfs, &mtdp_rootfs, 1);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":rootfs mtd driver registrated fail\n");
		return ret;
	}

#ifdef CONFIG_SXFLASH_ART
	/* ART sector initializing */
	ret = sxflash_init_sect(&sect_art, sxflash_art_sect, &flashop, 
						SXFLASH_SECT_READABLE | SXFLASH_SECT_WRITABLE,
						"art");
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":art sector init fail.\n");
		return ret;
	}

	ret = misc_register(&sxart_miscdev);
	if(ret < 0) {
		printk(KERN_ERR MODULE_NAME ":art driver registrated fail\n");
		return ret;
	}
#endif /* CONFIG_SXFLASH_ART */

	return 0;
}

void __exit sxflash_exit(void)
{
	misc_deregister(&sxeeprom_miscdev);
	misc_deregister(&sxapp_miscdev);
}

/**
 * EEPROM read/write API for Kernel developer
 *
 */
int sxflash_eplock(void)
{
	struct sectinfo *sectp = &sect_eeprom;

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	return 0;
}
EXPORT_SYMBOL(sxflash_eplock);

void sxflash_epunlock(void)
{
	struct sectinfo *sectp = &sect_eeprom;

	up(&sectp->lock);
}
EXPORT_SYMBOL(sxflash_epunlock);

int sxflash_epread(unsigned long offset, int size, unsigned char *buf)
{
	return sxflash_read_cache(&sect_eeprom, offset, size, buf);
}
EXPORT_SYMBOL(sxflash_epread);

u8 sxflash_epread8(unsigned long offset)
{
	u8 data;

	sxflash_epread(offset, sizeof(u8), (unsigned char*)&data);

	return data;
}
EXPORT_SYMBOL(sxflash_epread8);

u16 sxflash_epread16(unsigned long offset)
{
	u16 data;
	
	sxflash_epread(offset, sizeof(u16), (unsigned char*)&data);

	return be16_to_cpu(data);
}
EXPORT_SYMBOL(sxflash_epread16);

u32 sxflash_epread32(unsigned long offset)
{
	u32 data;

	sxflash_epread(offset, sizeof(u32), (unsigned char*)&data);

	return be32_to_cpu(data);
}
EXPORT_SYMBOL(sxflash_epread32);

void sxflash_set_epoverwrited(void)
{
	struct sectinfo *sectp = &sect_eeprom;

	sectp->flags |= SXFLASH_SECT_OVERWRITED;
}

ssize_t sxflash_epwrite(unsigned long offset, int size, unsigned char *buf)
{
	return sxflash_write_cache(&sect_eeprom, offset, size, buf);
}
EXPORT_SYMBOL(sxflash_epwrite);

void sxflash_epwrite8(unsigned long offset, u8 data)
{
	sxflash_epwrite(offset, sizeof(u8), (unsigned char*)&data);
}
EXPORT_SYMBOL(sxflash_epwrite8);

void sxflash_epwrite16(unsigned long offset, u16 data)
{
	data = cpu_to_be16(data);

	sxflash_epwrite(offset, sizeof(u16), (unsigned char*)&data);
}
EXPORT_SYMBOL(sxflash_epwrite16);

void sxflash_epwrite32(unsigned long offset, u32 data)
{
	data = cpu_to_be32(data);

	sxflash_epwrite(offset, sizeof(u32), (unsigned char*)&data);
}
EXPORT_SYMBOL(sxflash_epwrite32);

int sxflash_set_etheraddr(unsigned char *addr)
{
	int ret;
	u16 calc_sum;

	sxflash_eplock();
	calc_sum = sxflash_calc_checksum(addr, 6);
	calc_sum = be16_to_cpu(calc_sum); /* sxflash_epwrite have to set cpu endian */
	ret = sxflash_epwrite(SXFLASH_ETHER_ADDR_OFFSET, 6, addr);
	ret |= sxflash_epwrite(SXFLASH_ETHER_SUM_OFFSET, 2, (unsigned char*)&calc_sum);
	sxflash_set_epoverwrited();
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_set_etheraddr);

int sxflash_get_etheraddr(unsigned char *addr)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epread(SXFLASH_ETHER_ADDR_OFFSET, 6, addr);
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_get_etheraddr);

int sxflash_set_seriesname(unsigned char *name)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epwrite(SXFLASH_SERIES_NAME_OFFSET, 16, name);
	sxflash_set_epoverwrited();
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_set_seriesname);

int sxflash_get_seriesname(unsigned char *name)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epread(SXFLASH_SERIES_NAME_OFFSET, 16, name);
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_get_seriesname);

int sxflash_set_machinetype(unsigned char *type)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epwrite(SXFLASH_MACHINE_TYPE_OFFSET, 16, type);
	sxflash_set_epoverwrited();
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_set_machinetype);

int sxflash_get_machinetype(unsigned char *type)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epread(SXFLASH_MACHINE_TYPE_OFFSET, 16, type);
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_get_machinetype);

int sxflash_set_version(unsigned char *version)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epwrite(SXFLASH_VERSION_OFFSET, 16, version);
	sxflash_set_epoverwrited();
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_set_version);

int sxflash_get_version(unsigned char *version)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epread(SXFLASH_VERSION_OFFSET, 16, version);
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_get_version);

int sxflash_set_serialnum(unsigned char *serialnum)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epwrite(SXFLASH_SERIALNUM_OFFSET, 32, serialnum);
	sxflash_set_epoverwrited();
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_set_serialnum);

int sxflash_get_serialnum(unsigned char *serialnum)
{
	int ret;

	sxflash_eplock();
	ret = sxflash_epread(SXFLASH_SERIALNUM_OFFSET, 32, serialnum);
	sxflash_epunlock();

	return ret;
}
EXPORT_SYMBOL(sxflash_get_serialnum);


int sxflash_get_conflen(void)
{
	int len;

	sxflash_eplock();
	len = sxflash_epread32(SXFLASH_FILELEN_OFFSET);
	sxflash_epunlock();

	return len;
}
EXPORT_SYMBOL(sxflash_get_conflen);

/**
 * Get Is debug mode on of off
 *
 * @return On debug mode on, 1 is returned.
 * @return Off debug mode on, 0 is returned.
 */
int sxflash_get_is_debugmode(void)
{
	u32 flags;

	sxflash_eplock();
	flags = sxflash_epread32(SXFLASH_SYS_FLAGS_OFFSET);
	sxflash_epunlock();

	if(flags & SXFLASH_SYS_FLAG_DEBUG_DISABLE)
		return 0;
	else
		return 1;
}
EXPORT_SYMBOL(sxflash_get_is_debugmode);

/**
 * Set Debug mode on/off
 *
 * @param[enable] 0 = off, 1 = on
 *
 * @return On success, 0 is returned.
 * @return On error, a negative error number is returned.
 */
int sxflash_set_debugmode(int enable)
{
	int set_enable;
	u32 flags;

	set_enable = sxflash_get_is_debugmode();
	if(enable == set_enable) {
		/* Already set debug mode */
		return 0;
	}

	sxflash_eplock();
	flags = sxflash_epread32(SXFLASH_SYS_FLAGS_OFFSET);
	if(enable)
		flags &= ~SXFLASH_SYS_FLAG_DEBUG_DISABLE;
	else
		flags |= SXFLASH_SYS_FLAG_DEBUG_DISABLE;
	sxflash_epwrite32(SXFLASH_SYS_FLAGS_OFFSET, flags);
	sxflash_set_epoverwrited();
	sxflash_epunlock();

	return 0;
}
EXPORT_SYMBOL(sxflash_set_debugmode);


int sxflash_app_flush(void)
{
	struct sectinfo *sectp = &sect_app;
	int ret;

	if((sectp->flags & SXFLASH_SECT_OVERWRITED) == 0) {
		printk(KERN_INFO MODULE_NAME ":No change cache data\n");
		return 0;
	}

	if(down_interruptible(&sectp->lock))
		return -EINTR;

	ret = sxflash_check_cache(sectp);
	if(ret == 0) {
		ret = sxflash_save_cache(sectp);
	}

	up(&sectp->lock);

	return ret;
}


EXPORT_SYMBOL(sxflash_app_flush);

MODULE_LICENSE("GPL");

module_init(sxflash_init);
module_exit(sxflash_exit);
