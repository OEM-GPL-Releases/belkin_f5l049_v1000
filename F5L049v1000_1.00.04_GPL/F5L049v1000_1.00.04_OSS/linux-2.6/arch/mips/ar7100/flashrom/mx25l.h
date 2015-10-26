/**
 * @file
 *
 * MX25L Flash ROM driver.
 *
 * Copyright (C) 2008 - 2009 by silex technology, Inc.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef _ARCH_MX25L_H_
#define _ARCH_MX25L_H_

#include "sxflash.h"

#define FLASH_DEBUG      1

#define MX25L_BASE_ADDR  0xbf000000
#define F_ERS_DAT        0xffffffffL
#define F_END_FROM       NULL
#define FLASH_VERIFY_ON  1

#define SEND_COMMAND     8
#define SEND_ADDR        24
#define SEND_DATA        8
#define GET_DATA         8

/* COMMAND */
#define CMD_WREN         0x06
#define CMD_RD_STATUS    0x05
#define CMD_FAST_READ    0x0b
#define CMD_PAGE_PROG    0x02
#define CMD_BLOCK_ERASE  0xd8

/* Partitons */
extern F_BLOCK *sxflash_loader_sect[];
extern F_BLOCK *sxflash_eeprom0_sect[];
extern F_BLOCK *sxflash_eeprom1_sect[];
extern F_BLOCK *sxflash_app_sect[];
extern F_BLOCK *sxflash_rootfs_sect[];
extern F_BLOCK *sxflash_art_sect[];

int flash_mx25l_erase_sector(F_BLOCK **sectp);
int flash_mx25l_erase_block(F_BLOCK *blockp);
int flash_mx25l_read(F_BLOCK **blockp, unsigned long offset, unsigned int count, unsigned char *buf);
int flash_mx25l_write(F_BLOCK **blockp, unsigned long offset, unsigned int count, unsigned char *buf);
int flash_drv_init(struct sxflash_operations *flashop);

#endif /* _ARCH_MX25L_H_ */
