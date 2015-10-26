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
#ifndef _ASM_STI_FLASH_DEF
#define _ASM_STI_FLASH_DEF

#include <linux/autoconf.h>
#include <asm/page.h>

#define SXFLASH_IOC_MAGIC 'g' /* Checked no conflict int Documentation/ioctl-number.txt */

/*
 * IOCS - Set value from user space pointer
 * IOCG - Get value to user space pointer
 * IOCT - Tell command (no read/write from/to user space)
 * IOCX - eXchange value from/to user space pointer
 */
#define  SXFLASH_IOCS_FLUSHCACHE          _IO(SXFLASH_IOC_MAGIC, 0)
#define  SXFLASH_IOCS_SETDEFAULT          _IO(SXFLASH_IOC_MAGIC, 1)
#define  SXFLASH_IOCS_ETHEADDR            _IOW(SXFLASH_IOC_MAGIC, 2, int)
#define  SXFLASH_IOCG_ETHEADDR            _IOR(SXFLASH_IOC_MAGIC, 3, int)
#define  SXFLASH_IOCS_SERIES_NAME         _IOW(SXFLASH_IOC_MAGIC, 4, int)
#define  SXFLASH_IOCG_SERIES_NAME         _IOR(SXFLASH_IOC_MAGIC, 5, int)
#define  SXFLASH_IOCS_MACHINE_TYPE        _IOW(SXFLASH_IOC_MAGIC, 6, int)
#define  SXFLASH_IOCG_MACHINE_TYPE        _IOR(SXFLASH_IOC_MAGIC, 7, int)
#define  SXFLASH_IOCS_VERSION             _IOW(SXFLASH_IOC_MAGIC, 8, int)
#define  SXFLASH_IOCG_VERSION             _IOR(SXFLASH_IOC_MAGIC, 9, int)
#define  SXFLASH_IOCG_CONFLEN             _IOR(SXFLASH_IOC_MAGIC, 10, int)
#define  SXFLASH_IOCS_DEBUG               _IOW(SXFLASH_IOC_MAGIC, 12, int)
#define  SXFLASH_IOCG_DEBUG               _IOR(SXFLASH_IOC_MAGIC, 13, int)
#define  SXFLASH_IOCS_REGDOMAIN           _IOW(SXFLASH_IOC_MAGIC, 14, int)
#define  SXFLASH_IOCG_REGDOMAIN           _IOR(SXFLASH_IOC_MAGIC, 15, int)
#define  SXFLASH_IOCS_SERIALNUM           _IOW(SXFLASH_IOC_MAGIC, 16, int)
#define  SXFLASH_IOCG_SERIALNUM           _IOR(SXFLASH_IOC_MAGIC, 17, int)

#define  SXFLASH_MISC_DEVNUM_EEPROM       240
#define  SXFLASH_MISC_DEVNUM_APP          241
#define  SXFLASH_MISC_DEVNUM_ART          242

#ifdef __KERNEL__
/**********************************************/
/* SXFLASH global Define                      */
/**********************************************/
#define SXFLASH_CACHE_SIZE                PAGE_SIZE
#define SXFLASH_SECT_READABLE             0x00000001		/* Readable this sector (No effect now) */
#define SXFLASH_SECT_WRITABLE             0x00000002		/* Writable this sector (No effect now) */
#define SXFLASH_SECT_OVERWRITED           0x00000004		/* Overwrited and no flushed this sector */
#define SXFLASH_SECT_REVERSAL             0x00000008		/* Bit reversal (0->1 or 1->0) */

/**********************************************/
/* EEPROM Partition Information               */
/**********************************************/
#define SXFLASH_EEP_SUMINIT               0x55aa

#define SXFLASH_SYSTEM                    0
#define SXFLASH_SYS_SIZE                  512

#define SXFLASH_CONFIG                    (SXFLASH_SYSTEM + SXFLASH_SYS_SIZE)
#define SXFLASH_SYSTEMCONF_START          1024

/* Protect Area Offset */
#define SXFLASH_SYS_CHECKSUM_OFFSET       0x0000
#define SXFLASH_ETHER_ADDR_OFFSET         0x0002
#define SXFLASH_ETHER_SUM_OFFSET          0x0008
#define SXFLASH_SYS_FLAGS_OFFSET          0x000a
#define SXFLASH_SERIES_NAME_OFFSET        0x0020
#define SXFLASH_MACHINE_TYPE_OFFSET       0x0030
#define SXFLASH_VERSION_OFFSET            0x0040
#define SXFLASH_SERIALNUM_OFFSET          0x0050

/* Detail of SYS_FLAGS */
#define SXFLASH_SYS_FLAG_DEBUG_DISABLE    0x00000001    /* Serial, Telnet OFF */

/* Config area Header */
#define SXFLASH_CHECKSUM_OFFSET           0x0200
#define SXFLASH_WRITE_CNT_OFFSET          0x0204
#define SXFLASH_SETDEF_CNT_OFFSET         0x0208
#define SXFLASH_FILELEN_OFFSET            0x0210

/**********************************************/
/* Application Sector Information             */
/**********************************************/
#define SXFLASH_FIRMINFO_OFFSET           0x400
#define SXFLASH_FIRMINFO_SIZE             256

/**********************************************/
/* ART Partition Information                  */
/**********************************************/
#ifdef CONFIG_SXFLASH_ART
#define SXFLASH_ART_LENGTH_OFFSET         0x0200
#define SXFLASH_ART_CHECKSUM_OFFSET       0x0202
#define SXFLASH_ART_REGDOMAIN_OFFSET      0x0208
#define SXFLASH_ART_ETHER_ADDR_OFFSET     0x020C

#endif /* CONFIG_SXFLASH_ART */

typedef struct tag_f_block {
	unsigned int	offset;
	unsigned int	size;
} F_BLOCK;

struct sxflash_operations {
	/* Lock flash ROM */
	int (*lock)(void);
	/* Unlock flash ROM */
	void (*unlock)(void);
	/* erase blocks(sector) function */
	int (*erase_sect)(F_BLOCK **sectp);
	/* erase block function */
	int (*erase_block)(F_BLOCK *blockp);
	/* read flash function */
	int (*read)(F_BLOCK**, unsigned long, unsigned int, unsigned char*);
	/* write flash function */
	int (*write)(F_BLOCK**, unsigned long, unsigned int, unsigned char*);
};

struct sectinfo {
	char name[32];					/* Sector Name */
	unsigned int mode;				/* Mode setting for this sector */
	F_BLOCK **btablep;				/* Block Table (all block info in this sector) */
	unsigned int size;				/* Block size */
	unsigned char **cachep;			/* Sector backup data cache */
	int caches;						/* How many caches */
	unsigned long flags;			/* sector status and setting */
	struct semaphore lock;			/* Semaphore for this structure info */
	struct sxflash_operations *func; /* Flash standard operation func */
	int open_cnt;					 /* How many user opened this sector */
	int write_open_cnt;				 /* How many user write-opened this sector */
};

/* sxflash_init function */
int sxflash_initsect(struct sectinfo *sectp, F_BLOCK **btablep, 
								struct sxflash_operations *flashop, 
								unsigned long flags,
								const char *sectname);

/* EEPROM read/write API for Kernel developer */
int sxflash_eplock(void);
void sxflash_epunlock(void);

int sxflash_epread(unsigned long offset, int size, unsigned char *buf);
u8 sxflash_epread8(unsigned long offset);
u16 sxflash_epread16(unsigned long offset);
u32 sxflash_epread32(unsigned long offset);

void sxflash_set_epoverwrited(void);
ssize_t sxflash_epwrite(unsigned long offset, int size, unsigned char *buf);
void sxflash_epwrite8(unsigned long offset, u8 data);
void sxflash_epwrite16(unsigned long offset, u16 data);
void sxflash_epwrite32(unsigned long offset, u32 data);

int sxflash_set_etheraddr(unsigned char *addr);
int sxflash_get_etheraddr(unsigned char *addr);
int sxflash_set_seriesname(unsigned char *name);
int sxflash_get_seriesname(unsigned char *name);
int sxflash_set_machinetype(unsigned char *type);
int sxflash_get_machinetype(unsigned char *type);
int sxflash_set_version(unsigned char *version);
int sxflash_get_version(unsigned char *version);
int sxflash_set_serialnum(unsigned char *serialnum);
int sxflash_get_serialnum(unsigned char *serialnum);
int sxflash_get_conflen(void);
int sxflash_set_debugmode(int enable);
int sxflash_get_is_debugmode(void);
int sxflash_app_flush(void);

#endif /* __KERNEL__ */


#endif /* _ASM_STI_FLASH_DEF */

