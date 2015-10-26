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

#include <linux/string.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <asm/semaphore.h>
#include <asm/byteorder.h>

#include <asm/mach-ar7100/ar7100.h>

#include "mx25l.h"
#include "sxflash.h"

#undef DEBUG 

static struct semaphore flash_sem;

/* Flash memory sector offset(16MByte) ------------------------------ */
F_BLOCK flash_block[] = {
	{ 0x00000000L, 65536 },    /* 64Kbyte[00] */
	{ 0x00010000L, 65536 },    /* 64Kbyte[01] */
	{ 0x00020000L, 65536 },    /* 64Kbyte[02] */
	{ 0x00030000L, 65536 },    /* 64Kbyte[03] */
	{ 0x00040000L, 65536 },    /* 64Kbyte[04] */
	{ 0x00050000L, 65536 },    /* 64Kbyte[05] */
	{ 0x00060000L, 65536 },    /* 64Kbyte[06] */
	{ 0x00070000L, 65536 },    /* 64Kbyte[07] */
	{ 0x00080000L, 65536 },    /* 64Kbyte[08] */
	{ 0x00090000L, 65536 },    /* 64Kbyte[09] */
	{ 0x000a0000L, 65536 },    /* 64Kbyte[10] */
	{ 0x000b0000L, 65536 },    /* 64Kbyte[11] */
	{ 0x000c0000L, 65536 },    /* 64Kbyte[12] */
	{ 0x000d0000L, 65536 },    /* 64Kbyte[13] */
	{ 0x000e0000L, 65536 },    /* 64Kbyte[14] */
	{ 0x000f0000L, 65536 },    /* 64Kbyte[15] */
	{ 0x00100000L, 65536 },    /* 64Kbyte[16] */
	{ 0x00110000L, 65536 },    /* 64Kbyte[17] */
	{ 0x00120000L, 65536 },    /* 64Kbyte[18] */
	{ 0x00130000L, 65536 },    /* 64Kbyte[19] */
	{ 0x00140000L, 65536 },    /* 64Kbyte[20] */
	{ 0x00150000L, 65536 },    /* 64Kbyte[21] */
	{ 0x00160000L, 65536 },    /* 64Kbyte[22] */
	{ 0x00170000L, 65536 },    /* 64Kbyte[23] */
	{ 0x00180000L, 65536 },    /* 64Kbyte[24] */
	{ 0x00190000L, 65536 },    /* 64Kbyte[25] */
	{ 0x001a0000L, 65536 },    /* 64Kbyte[26] */
	{ 0x001b0000L, 65536 },    /* 64Kbyte[27] */
	{ 0x001c0000L, 65536 },    /* 64Kbyte[28] */
	{ 0x001d0000L, 65536 },    /* 64Kbyte[29] */
	{ 0x001e0000L, 65536 },    /* 64Kbyte[30] */
	{ 0x001f0000L, 65536 },    /* 64Kbyte[31] */
	{ 0x00200000L, 65536 },    /* 64Kbyte[32] */
	{ 0x00210000L, 65536 },    /* 64Kbyte[33] */
	{ 0x00220000L, 65536 },    /* 64Kbyte[34] */
	{ 0x00230000L, 65536 },    /* 64Kbyte[35] */
	{ 0x00240000L, 65536 },    /* 64Kbyte[36] */
	{ 0x00250000L, 65536 },    /* 64Kbyte[37] */
	{ 0x00260000L, 65536 },    /* 64Kbyte[38] */
	{ 0x00270000L, 65536 },    /* 64Kbyte[39] */
	{ 0x00280000L, 65536 },    /* 64Kbyte[40] */
	{ 0x00290000L, 65536 },    /* 64Kbyte[41] */
	{ 0x002a0000L, 65536 },    /* 64Kbyte[42] */
	{ 0x002b0000L, 65536 },    /* 64Kbyte[43] */
	{ 0x002c0000L, 65536 },    /* 64Kbyte[44] */
	{ 0x002d0000L, 65536 },    /* 64Kbyte[45] */
	{ 0x002e0000L, 65536 },    /* 64Kbyte[46] */
	{ 0x002f0000L, 65536 },    /* 64Kbyte[47] */
	{ 0x00300000L, 65536 },    /* 64Kbyte[48] */
	{ 0x00310000L, 65536 },    /* 64Kbyte[49] */
	{ 0x00320000L, 65536 },    /* 64Kbyte[50] */
	{ 0x00330000L, 65536 },    /* 64Kbyte[51] */
	{ 0x00340000L, 65536 },    /* 64Kbyte[52] */
	{ 0x00350000L, 65536 },    /* 64Kbyte[53] */
	{ 0x00360000L, 65536 },    /* 64Kbyte[54] */
	{ 0x00370000L, 65536 },    /* 64Kbyte[55] */
	{ 0x00380000L, 65536 },    /* 64Kbyte[56] */
	{ 0x00390000L, 65536 },    /* 64Kbyte[57] */
	{ 0x003a0000L, 65536 },    /* 64Kbyte[58] */
	{ 0x003b0000L, 65536 },    /* 64Kbyte[59] */
	{ 0x003c0000L, 65536 },    /* 64Kbyte[60] */
	{ 0x003d0000L, 65536 },    /* 64Kbyte[61] */
	{ 0x003e0000L, 65536 },    /* 64Kbyte[62] */
	{ 0x003f0000L, 65536 },    /* 64Kbyte[63] */
	{ 0x00400000L, 65536 },    /* 64Kbyte[64] */
	{ 0x00410000L, 65536 },    /* 64Kbyte[65] */
	{ 0x00420000L, 65536 },    /* 64Kbyte[66] */
	{ 0x00430000L, 65536 },    /* 64Kbyte[67] */
	{ 0x00440000L, 65536 },    /* 64Kbyte[68] */
	{ 0x00450000L, 65536 },    /* 64Kbyte[69] */
	{ 0x00460000L, 65536 },    /* 64Kbyte[70] */
	{ 0x00470000L, 65536 },    /* 64Kbyte[71] */
	{ 0x00480000L, 65536 },    /* 64Kbyte[72] */
	{ 0x00490000L, 65536 },    /* 64Kbyte[73] */
	{ 0x004a0000L, 65536 },    /* 64Kbyte[74] */
	{ 0x004b0000L, 65536 },    /* 64Kbyte[75] */
	{ 0x004c0000L, 65536 },    /* 64Kbyte[76] */
	{ 0x004d0000L, 65536 },    /* 64Kbyte[77] */
	{ 0x004e0000L, 65536 },    /* 64Kbyte[78] */
	{ 0x004f0000L, 65536 },    /* 64Kbyte[79] */
	{ 0x00500000L, 65536 },    /* 64Kbyte[80] */
	{ 0x00510000L, 65536 },    /* 64Kbyte[81] */
	{ 0x00520000L, 65536 },    /* 64Kbyte[82] */
	{ 0x00530000L, 65536 },    /* 64Kbyte[83] */
	{ 0x00540000L, 65536 },    /* 64Kbyte[84] */
	{ 0x00550000L, 65536 },    /* 64Kbyte[85] */
	{ 0x00560000L, 65536 },    /* 64Kbyte[86] */
	{ 0x00570000L, 65536 },    /* 64Kbyte[87] */
	{ 0x00580000L, 65536 },    /* 64Kbyte[88] */
	{ 0x00590000L, 65536 },    /* 64Kbyte[89] */
	{ 0x005a0000L, 65536 },    /* 64Kbyte[90] */
	{ 0x005b0000L, 65536 },    /* 64Kbyte[91] */
	{ 0x005c0000L, 65536 },    /* 64Kbyte[92] */
	{ 0x005d0000L, 65536 },    /* 64Kbyte[93] */
	{ 0x005e0000L, 65536 },    /* 64Kbyte[94] */
	{ 0x005f0000L, 65536 },    /* 64Kbyte[95] */
	{ 0x00600000L, 65536 },    /* 64Kbyte[96] */
	{ 0x00610000L, 65536 },    /* 64Kbyte[97] */
	{ 0x00620000L, 65536 },    /* 64Kbyte[98] */
	{ 0x00630000L, 65536 },    /* 64Kbyte[99] */
	{ 0x00640000L, 65536 },    /* 64Kbyte[100] */
	{ 0x00650000L, 65536 },    /* 64Kbyte[101] */
	{ 0x00660000L, 65536 },    /* 64Kbyte[102] */
	{ 0x00670000L, 65536 },    /* 64Kbyte[103] */
	{ 0x00680000L, 65536 },    /* 64Kbyte[104] */
	{ 0x00690000L, 65536 },    /* 64Kbyte[105] */
	{ 0x006a0000L, 65536 },    /* 64Kbyte[106] */
	{ 0x006b0000L, 65536 },    /* 64Kbyte[107] */
	{ 0x006c0000L, 65536 },    /* 64Kbyte[108] */
	{ 0x006d0000L, 65536 },    /* 64Kbyte[109] */
	{ 0x006e0000L, 65536 },    /* 64Kbyte[110] */
	{ 0x006f0000L, 65536 },    /* 64Kbyte[111] */
	{ 0x00700000L, 65536 },    /* 64Kbyte[112] */
	{ 0x00710000L, 65536 },    /* 64Kbyte[113] */
	{ 0x00720000L, 65536 },    /* 64Kbyte[114] */
	{ 0x00730000L, 65536 },    /* 64Kbyte[115] */
	{ 0x00740000L, 65536 },    /* 64Kbyte[116] */
	{ 0x00750000L, 65536 },    /* 64Kbyte[117] */
	{ 0x00760000L, 65536 },    /* 64Kbyte[118] */
	{ 0x00770000L, 65536 },    /* 64Kbyte[119] */
	{ 0x00780000L, 65536 },    /* 64Kbyte[120] */
	{ 0x00790000L, 65536 },    /* 64Kbyte[121] */
	{ 0x007a0000L, 65536 },    /* 64Kbyte[122] */
	{ 0x007b0000L, 65536 },    /* 64Kbyte[123] */
	{ 0x007c0000L, 65536 },    /* 64Kbyte[124] */
	{ 0x007d0000L, 65536 },    /* 64Kbyte[125] */
	{ 0x007e0000L, 65536 },    /* 64Kbyte[126] */
	{ 0x007f0000L, 65536 },    /* 64Kbyte[127] */
	{ 0x00800000L, 65536 },    /* 64Kbyte[128] */
	{ 0x00810000L, 65536 },    /* 64Kbyte[129] */
	{ 0x00820000L, 65536 },    /* 64Kbyte[130] */
	{ 0x00830000L, 65536 },    /* 64Kbyte[131] */
	{ 0x00840000L, 65536 },    /* 64Kbyte[132] */
	{ 0x00850000L, 65536 },    /* 64Kbyte[133] */
	{ 0x00860000L, 65536 },    /* 64Kbyte[134] */
	{ 0x00870000L, 65536 },    /* 64Kbyte[135] */
	{ 0x00880000L, 65536 },    /* 64Kbyte[136] */
	{ 0x00890000L, 65536 },    /* 64Kbyte[137] */
	{ 0x008a0000L, 65536 },    /* 64Kbyte[138] */
	{ 0x008b0000L, 65536 },    /* 64Kbyte[139] */
	{ 0x008c0000L, 65536 },    /* 64Kbyte[140] */
	{ 0x008d0000L, 65536 },    /* 64Kbyte[141] */
	{ 0x008e0000L, 65536 },    /* 64Kbyte[142] */
	{ 0x008f0000L, 65536 },    /* 64Kbyte[143] */
	{ 0x00900000L, 65536 },    /* 64Kbyte[144] */
	{ 0x00910000L, 65536 },    /* 64Kbyte[145] */
	{ 0x00920000L, 65536 },    /* 64Kbyte[146] */
	{ 0x00930000L, 65536 },    /* 64Kbyte[147] */
	{ 0x00940000L, 65536 },    /* 64Kbyte[148] */
	{ 0x00950000L, 65536 },    /* 64Kbyte[149] */
	{ 0x00960000L, 65536 },    /* 64Kbyte[150] */
	{ 0x00970000L, 65536 },    /* 64Kbyte[151] */
	{ 0x00980000L, 65536 },    /* 64Kbyte[152] */
	{ 0x00990000L, 65536 },    /* 64Kbyte[153] */
	{ 0x009a0000L, 65536 },    /* 64Kbyte[154] */
	{ 0x009b0000L, 65536 },    /* 64Kbyte[155] */
	{ 0x009c0000L, 65536 },    /* 64Kbyte[156] */
	{ 0x009d0000L, 65536 },    /* 64Kbyte[157] */
	{ 0x009e0000L, 65536 },    /* 64Kbyte[158] */
	{ 0x009f0000L, 65536 },    /* 64Kbyte[159] */
	{ 0x00a00000L, 65536 },    /* 64Kbyte[160] */
	{ 0x00a10000L, 65536 },    /* 64Kbyte[161] */
	{ 0x00a20000L, 65536 },    /* 64Kbyte[162] */
	{ 0x00a30000L, 65536 },    /* 64Kbyte[163] */
	{ 0x00a40000L, 65536 },    /* 64Kbyte[164] */
	{ 0x00a50000L, 65536 },    /* 64Kbyte[165] */
	{ 0x00a60000L, 65536 },    /* 64Kbyte[166] */
	{ 0x00a70000L, 65536 },    /* 64Kbyte[167] */
	{ 0x00a80000L, 65536 },    /* 64Kbyte[168] */
	{ 0x00a90000L, 65536 },    /* 64Kbyte[169] */
	{ 0x00aa0000L, 65536 },    /* 64Kbyte[170] */
	{ 0x00ab0000L, 65536 },    /* 64Kbyte[171] */
	{ 0x00ac0000L, 65536 },    /* 64Kbyte[172] */
	{ 0x00ad0000L, 65536 },    /* 64Kbyte[173] */
	{ 0x00ae0000L, 65536 },    /* 64Kbyte[174] */
	{ 0x00af0000L, 65536 },    /* 64Kbyte[175] */
	{ 0x00b00000L, 65536 },    /* 64Kbyte[176] */
	{ 0x00b10000L, 65536 },    /* 64Kbyte[177] */
	{ 0x00b20000L, 65536 },    /* 64Kbyte[178] */
	{ 0x00b30000L, 65536 },    /* 64Kbyte[179] */
	{ 0x00b40000L, 65536 },    /* 64Kbyte[180] */
	{ 0x00b50000L, 65536 },    /* 64Kbyte[181] */
	{ 0x00b60000L, 65536 },    /* 64Kbyte[182] */
	{ 0x00b70000L, 65536 },    /* 64Kbyte[183] */
	{ 0x00b80000L, 65536 },    /* 64Kbyte[184] */
	{ 0x00b90000L, 65536 },    /* 64Kbyte[185] */
	{ 0x00ba0000L, 65536 },    /* 64Kbyte[186] */
	{ 0x00bb0000L, 65536 },    /* 64Kbyte[187] */
	{ 0x00bc0000L, 65536 },    /* 64Kbyte[188] */
	{ 0x00bd0000L, 65536 },    /* 64Kbyte[189] */
	{ 0x00be0000L, 65536 },    /* 64Kbyte[190] */
	{ 0x00bf0000L, 65536 },    /* 64Kbyte[191] */
	{ 0x00c00000L, 65536 },    /* 64Kbyte[192] */
	{ 0x00c10000L, 65536 },    /* 64Kbyte[193] */
	{ 0x00c20000L, 65536 },    /* 64Kbyte[194] */
	{ 0x00c30000L, 65536 },    /* 64Kbyte[195] */
	{ 0x00c40000L, 65536 },    /* 64Kbyte[196] */
	{ 0x00c50000L, 65536 },    /* 64Kbyte[197] */
	{ 0x00c60000L, 65536 },    /* 64Kbyte[198] */
	{ 0x00c70000L, 65536 },    /* 64Kbyte[199] */
	{ 0x00c80000L, 65536 },    /* 64Kbyte[200] */
	{ 0x00c90000L, 65536 },    /* 64Kbyte[201] */
	{ 0x00ca0000L, 65536 },    /* 64Kbyte[202] */
	{ 0x00cb0000L, 65536 },    /* 64Kbyte[203] */
	{ 0x00cc0000L, 65536 },    /* 64Kbyte[204] */
	{ 0x00cd0000L, 65536 },    /* 64Kbyte[205] */
	{ 0x00ce0000L, 65536 },    /* 64Kbyte[206] */
	{ 0x00cf0000L, 65536 },    /* 64Kbyte[207] */
	{ 0x00d00000L, 65536 },    /* 64Kbyte[208] */
	{ 0x00d10000L, 65536 },    /* 64Kbyte[209] */
	{ 0x00d20000L, 65536 },    /* 64Kbyte[210] */
	{ 0x00d30000L, 65536 },    /* 64Kbyte[211] */
	{ 0x00d40000L, 65536 },    /* 64Kbyte[212] */
	{ 0x00d50000L, 65536 },    /* 64Kbyte[213] */
	{ 0x00d60000L, 65536 },    /* 64Kbyte[214] */
	{ 0x00d70000L, 65536 },    /* 64Kbyte[215] */
	{ 0x00d80000L, 65536 },    /* 64Kbyte[216] */
	{ 0x00d90000L, 65536 },    /* 64Kbyte[217] */
	{ 0x00da0000L, 65536 },    /* 64Kbyte[218] */
	{ 0x00db0000L, 65536 },    /* 64Kbyte[219] */
	{ 0x00dc0000L, 65536 },    /* 64Kbyte[220] */
	{ 0x00dd0000L, 65536 },    /* 64Kbyte[221] */
	{ 0x00de0000L, 65536 },    /* 64Kbyte[222] */
	{ 0x00df0000L, 65536 },    /* 64Kbyte[223] */
	{ 0x00e00000L, 65536 },    /* 64Kbyte[224] */
	{ 0x00e10000L, 65536 },    /* 64Kbyte[225] */
	{ 0x00e20000L, 65536 },    /* 64Kbyte[226] */
	{ 0x00e30000L, 65536 },    /* 64Kbyte[227] */
	{ 0x00e40000L, 65536 },    /* 64Kbyte[228] */
	{ 0x00e50000L, 65536 },    /* 64Kbyte[229] */
	{ 0x00e60000L, 65536 },    /* 64Kbyte[230] */
	{ 0x00e70000L, 65536 },    /* 64Kbyte[231] */
	{ 0x00e80000L, 65536 },    /* 64Kbyte[232] */
	{ 0x00e90000L, 65536 },    /* 64Kbyte[233] */
	{ 0x00ea0000L, 65536 },    /* 64Kbyte[234] */
	{ 0x00eb0000L, 65536 },    /* 64Kbyte[235] */
	{ 0x00ec0000L, 65536 },    /* 64Kbyte[236] */
	{ 0x00ed0000L, 65536 },    /* 64Kbyte[237] */
	{ 0x00ee0000L, 65536 },    /* 64Kbyte[238] */
	{ 0x00ef0000L, 65536 },    /* 64Kbyte[239] */
	{ 0x00f00000L, 65536 },    /* 64Kbyte[240] */
	{ 0x00f10000L, 65536 },    /* 64Kbyte[241] */
	{ 0x00f20000L, 65536 },    /* 64Kbyte[242] */
	{ 0x00f30000L, 65536 },    /* 64Kbyte[243] */
	{ 0x00f40000L, 65536 },    /* 64Kbyte[244] */
	{ 0x00f50000L, 65536 },    /* 64Kbyte[245] */
	{ 0x00f60000L, 65536 },    /* 64Kbyte[246] */
	{ 0x00f70000L, 65536 },    /* 64Kbyte[247] */
	{ 0x00f80000L, 65536 },    /* 64Kbyte[248] */
	{ 0x00f90000L, 65536 },    /* 64Kbyte[249] */
	{ 0x00fa0000L, 65536 },    /* 64Kbyte[250] */
	{ 0x00fb0000L, 65536 },    /* 64Kbyte[251] */
	{ 0x00fc0000L, 65536 },    /* 64Kbyte[252] */
	{ 0x00fd0000L, 65536 },    /* 64Kbyte[253] */
	{ 0x00fe0000L, 65536 },    /* 64Kbyte[254] */
	{ 0x00ff0000L, 65536 },    /* 64Kbyte[255] */
	{ 0x00000000L, 0  }
};

F_BLOCK flash_2m_end  = { 0x00200000L,  0 };
F_BLOCK flash_4m_end  = { 0x00400000L,  0 };
F_BLOCK flash_6m_end  = { 0x00600000L,  0 };
F_BLOCK flash_8m_end  = { 0x00800000L,  0 };
F_BLOCK flash_16m_end = { 0x01000000L,  0 };

/* Loader Space */
F_BLOCK *sxflash_loader_sect[] =
{
	&flash_block[0],
	&flash_block[1],
	F_END_FROM
};

/* EEPROM(1) Space */
F_BLOCK *sxflash_eeprom0_sect[] =
{
	&flash_block[2],
	&flash_block[3],
	&flash_block[4],
	&flash_block[5],
	&flash_block[6],
	&flash_block[7],
	&flash_block[8],
	&flash_block[9],
	F_END_FROM
};

/* EEPROM(2) Space */
F_BLOCK *sxflash_eeprom1_sect[] =
{
	&flash_block[10],
	&flash_block[11],
	&flash_block[12],
	&flash_block[13],
	&flash_block[14],
	&flash_block[15],
	&flash_block[16],
	&flash_block[17],
	F_END_FROM
};

/* Application Space */
F_BLOCK *sxflash_app_sect[] =
{
	&flash_block[18],
	&flash_block[19],
	&flash_block[20],
	&flash_block[21],
	&flash_block[22],
	&flash_block[23],
	&flash_block[24],
	&flash_block[25],
	&flash_block[26],
	&flash_block[27],
	&flash_block[28],
	&flash_block[29],
	&flash_block[30],
	&flash_block[31],
	&flash_block[32],
	&flash_block[33],
	&flash_block[34],
	&flash_block[35],
	&flash_block[36],
	&flash_block[37],
	&flash_block[38],
	&flash_block[39],
	&flash_block[40],
	&flash_block[41],
	&flash_block[42],
	&flash_block[43],
	&flash_block[44],
	&flash_block[45],
	&flash_block[46],
	&flash_block[47],
	&flash_block[48],
	&flash_block[49],
	&flash_block[50],
	&flash_block[51],
	&flash_block[52],
	&flash_block[53],
	&flash_block[54],
	&flash_block[55],
	&flash_block[56],
	&flash_block[57],
	&flash_block[58],
	&flash_block[59],
	&flash_block[60],
	&flash_block[61],
	&flash_block[62],
	&flash_block[63],
	&flash_block[64],
	&flash_block[65],
	&flash_block[66],
	&flash_block[67],
	&flash_block[68],
	&flash_block[69],
	&flash_block[70],
	&flash_block[71],
	&flash_block[72],
	&flash_block[73],
	&flash_block[74],
	&flash_block[75],
	&flash_block[76],
	&flash_block[77],
	&flash_block[78],
	&flash_block[79],
	&flash_block[80],
	&flash_block[81],
	&flash_block[82],
	&flash_block[83],
	&flash_block[84],
	&flash_block[85],
	&flash_block[86],
	&flash_block[87],
	&flash_block[88],
	&flash_block[89],
	&flash_block[90],
	&flash_block[91],
	&flash_block[92],
	&flash_block[93],
	&flash_block[94],
	&flash_block[95],
	&flash_block[96],
	&flash_block[97],
	&flash_block[98],
	&flash_block[99],
	&flash_block[100],
	&flash_block[101],
	&flash_block[102],
	&flash_block[103],
	&flash_block[104],
	&flash_block[105],
	&flash_block[106],
	&flash_block[107],
	&flash_block[108],
	&flash_block[109],
	&flash_block[110],
	&flash_block[111],
	&flash_block[112],
	&flash_block[113],
	&flash_block[114],
	&flash_block[115],
	&flash_block[116],
	&flash_block[117],
	&flash_block[118],
	&flash_block[119],
	&flash_block[120],
	&flash_block[121],
	&flash_block[122],
	&flash_block[123],
	&flash_block[124],
	&flash_block[125],
	&flash_block[126],
	&flash_block[127],
	&flash_block[128],
	&flash_block[129],
	&flash_block[130],
	&flash_block[131],
	&flash_block[132],
	&flash_block[133],
	&flash_block[134],
	&flash_block[135],
	&flash_block[136],
	&flash_block[137],
	&flash_block[138],
	&flash_block[139],
	&flash_block[140],
	&flash_block[141],
	&flash_block[142],
	&flash_block[143],
	&flash_block[144],
	&flash_block[145],
	&flash_block[146],
	&flash_block[147],
	&flash_block[148],
	&flash_block[149],
	&flash_block[150],
	&flash_block[151],
	&flash_block[152],
	&flash_block[153],
	&flash_block[154],
	&flash_block[155],
	&flash_block[156],
	&flash_block[157],
	&flash_block[158],
	&flash_block[159],
	&flash_block[160],
	&flash_block[161],
	&flash_block[162],
	&flash_block[163],
	&flash_block[164],
	&flash_block[165],
	&flash_block[166],
	&flash_block[167],
	&flash_block[168],
	&flash_block[169],
	&flash_block[170],
	&flash_block[171],
	&flash_block[172],
	&flash_block[173],
	&flash_block[174],
	&flash_block[175],
	&flash_block[176],
	&flash_block[177],
	&flash_block[178],
	&flash_block[179],
	&flash_block[180],
	&flash_block[181],
	&flash_block[182],
	&flash_block[183],
	&flash_block[184],
	&flash_block[185],
	&flash_block[186],
	&flash_block[187],
	&flash_block[188],
	&flash_block[189],
	&flash_block[190],
	&flash_block[191],
	&flash_block[192],
	&flash_block[193],
	&flash_block[194],
	&flash_block[195],
	&flash_block[196],
	&flash_block[197],
	&flash_block[198],
	&flash_block[199],
	&flash_block[200],
	&flash_block[201],
	&flash_block[202],
	&flash_block[203],
	&flash_block[204],
	&flash_block[205],
	&flash_block[206],
	&flash_block[207],
	&flash_block[208],
	&flash_block[209],
	&flash_block[210],
	&flash_block[211],
	&flash_block[212],
	&flash_block[213],
	&flash_block[214],
	&flash_block[215],
	&flash_block[216],
	&flash_block[217],
	&flash_block[218],
	&flash_block[219],
	&flash_block[220],
	&flash_block[221],
	&flash_block[222],
	&flash_block[223],
	&flash_block[224],
	&flash_block[225],
	&flash_block[226],
	&flash_block[227],
	&flash_block[228],
	&flash_block[229],
	&flash_block[230],
	&flash_block[231],
	&flash_block[232],
	&flash_block[233],
	&flash_block[234],
	&flash_block[235],
	&flash_block[236],
	&flash_block[237],
	&flash_block[238],
	&flash_block[239],
	&flash_block[240],
	&flash_block[241],
	&flash_block[242],
	&flash_block[243],
	&flash_block[244],
	&flash_block[245],
	&flash_block[246],
	&flash_block[247],
	&flash_block[248],
	&flash_block[249],
	&flash_block[250],
	&flash_block[251],
	&flash_block[252],
	&flash_block[253],
	&flash_block[254],
	F_END_FROM
};

/* Rootfs Space */
F_BLOCK *sxflash_rootfs_sect[] =
{
	&flash_block[42],
	&flash_block[43],
	&flash_block[44],
	&flash_block[45],
	&flash_block[46],
	&flash_block[47],
	&flash_block[48],
	&flash_block[49],
	&flash_block[50],
	&flash_block[51],
	&flash_block[52],
	&flash_block[53],
	&flash_block[54],
	&flash_block[55],
	&flash_block[56],
	&flash_block[57],
	&flash_block[58],
	&flash_block[59],
	&flash_block[60],
	&flash_block[61],
	&flash_block[62],
	&flash_block[63],
	&flash_block[64],
	&flash_block[65],
	&flash_block[66],
	&flash_block[67],
	&flash_block[68],
	&flash_block[69],
	&flash_block[70],
	&flash_block[71],
	&flash_block[72],
	&flash_block[73],
	&flash_block[74],
	&flash_block[75],
	&flash_block[76],
	&flash_block[77],
	&flash_block[78],
	&flash_block[79],
	&flash_block[80],
	&flash_block[81],
	&flash_block[82],
	&flash_block[83],
	&flash_block[84],
	&flash_block[85],
	&flash_block[86],
	&flash_block[87],
	&flash_block[88],
	&flash_block[89],
	&flash_block[90],
	&flash_block[91],
	&flash_block[92],
	&flash_block[93],
	&flash_block[94],
	&flash_block[95],
	&flash_block[96],
	&flash_block[97],
	&flash_block[98],
	&flash_block[99],
	&flash_block[100],
	&flash_block[101],
	&flash_block[102],
	&flash_block[103],
	&flash_block[104],
	&flash_block[105],
	&flash_block[106],
	&flash_block[107],
	&flash_block[108],
	&flash_block[109],
	&flash_block[110],
	&flash_block[111],
	&flash_block[112],
	&flash_block[113],
	&flash_block[114],
	&flash_block[115],
	&flash_block[116],
	&flash_block[117],
	&flash_block[118],
	&flash_block[119],
	&flash_block[120],
	&flash_block[121],
	&flash_block[122],
	&flash_block[123],
	&flash_block[124],
	&flash_block[125],
	&flash_block[126],
	&flash_block[127],
	&flash_block[128],
	&flash_block[129],
	&flash_block[130],
	&flash_block[131],
	&flash_block[132],
	&flash_block[133],
	&flash_block[134],
	&flash_block[135],
	&flash_block[136],
	&flash_block[137],
	&flash_block[138],
	&flash_block[139],
	&flash_block[140],
	&flash_block[141],
	&flash_block[142],
	&flash_block[143],
	&flash_block[144],
	&flash_block[145],
	&flash_block[146],
	&flash_block[147],
	&flash_block[148],
	&flash_block[149],
	&flash_block[150],
	&flash_block[151],
	&flash_block[152],
	&flash_block[153],
	&flash_block[154],
	&flash_block[155],
	&flash_block[156],
	&flash_block[157],
	&flash_block[158],
	&flash_block[159],
	&flash_block[160],
	&flash_block[161],
	&flash_block[162],
	&flash_block[163],
	&flash_block[164],
	&flash_block[165],
	&flash_block[166],
	&flash_block[167],
	&flash_block[168],
	&flash_block[169],
	&flash_block[170],
	&flash_block[171],
	&flash_block[172],
	&flash_block[173],
	&flash_block[174],
	&flash_block[175],
	&flash_block[176],
	&flash_block[177],
	&flash_block[178],
	&flash_block[179],
	&flash_block[180],
	&flash_block[181],
	&flash_block[182],
	&flash_block[183],
	&flash_block[184],
	&flash_block[185],
	&flash_block[186],
	&flash_block[187],
	&flash_block[188],
	&flash_block[189],
	&flash_block[190],
	&flash_block[191],
	&flash_block[192],
	&flash_block[193],
	&flash_block[194],
	&flash_block[195],
	&flash_block[196],
	&flash_block[197],
	&flash_block[198],
	&flash_block[199],
	&flash_block[200],
	&flash_block[201],
	&flash_block[202],
	&flash_block[203],
	&flash_block[204],
	&flash_block[205],
	&flash_block[206],
	&flash_block[207],
	&flash_block[208],
	&flash_block[209],
	&flash_block[210],
	&flash_block[211],
	&flash_block[212],
	&flash_block[213],
	&flash_block[214],
	&flash_block[215],
	&flash_block[216],
	&flash_block[217],
	&flash_block[218],
	&flash_block[219],
	&flash_block[220],
	&flash_block[221],
	&flash_block[222],
	&flash_block[223],
	&flash_block[224],
	&flash_block[225],
	&flash_block[226],
	&flash_block[227],
	&flash_block[228],
	&flash_block[229],
	&flash_block[230],
	&flash_block[231],
	&flash_block[232],
	&flash_block[233],
	&flash_block[234],
	&flash_block[235],
	&flash_block[236],
	&flash_block[237],
	&flash_block[238],
	&flash_block[239],
	&flash_block[240],
	&flash_block[241],
	&flash_block[242],
	&flash_block[243],
	&flash_block[244],
	&flash_block[245],
	&flash_block[246],
	&flash_block[247],
	&flash_block[248],
	&flash_block[249],
	&flash_block[250],
	&flash_block[251],
	&flash_block[252],
	&flash_block[253],
	&flash_block[254],
	F_END_FROM
};

F_BLOCK *sxflash_art_sect[] =
{
	&flash_block[255],
	F_END_FROM
};

/* Flash Memory lock ------------------------------------------------------ */
static int flash_mx25l_lock(void)
{
	down(&flash_sem);
	return 0;
}

/* Flash Memory Unlock ---------------------------------------------------- */
static void flash_mx25l_unlock(void)
{
	up(&flash_sem);
}

/* Flash Function Select -------------------------------------------------- */
static void flash_func_select(int enable)
{
	/*
	 * Function Select bit:
	 * When this bit is set to 0, reading from this register reads the contents
	 * of the serial flash. Writes to locations in other SPI registers have no
	 * effect if this bit is set to 0.
	 * Writing a one value to this bit selects GPIO mode for the SPI controller.
	 * In GPIO mode, other SPI registers are visible. (by MX25L12805D)
	 */
	ar7100_reg_wr_nf(AR7100_SPI_FS ,enable);
}

/* Flash SPI I/O Bit Bangout ---------------------------------------------- */
static void flash_bit_bangout(u32 data, int count)
{
	u32 mask, data_bit;

	mask = 1 << (count - 1);

	/* Write xbit */
	do {
		/* DO pin */
		if (data & mask) data_bit = 0x00000001;
		else             data_bit = 0x00000000;

		/* CS_0=0(En), CLK=0, and DO */
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, 0x00060000 | data_bit);

		/* Input data is latched on the rising edge of Serial Clock(SCLK)   */
		/* and data shifts out on the falling edge of SCLK.(by MX25L12805D) */

		/* CS_0=0(En), CLK=1. and DO */
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, 0x00060100 | data_bit);

		mask = mask >> 1;
	} while (mask);
}

/* Flash SPI I/O Bit Bangout Done ----------------------------------------- */
static void flash_bit_bangout_done(void)
{
	/* CS_0=0(En), CLK=0, DO=0 */
	ar7100_reg_wr_nf(AR7100_SPI_WRITE, 0x00060000);

	/* CS_0=1(Dis) */
	ar7100_reg_wr_nf(AR7100_SPI_WRITE, 0x00070000);
}

/* Flash Write Enable Command --------------------------------------------- */
static void flash_write_enable(void)
{
	/* 
	 * The Write Enable instructions is for setting Write Enable Latch(WEL) bit.
	 * The WEL is reset by following situations:
	 * -Power-up
	 * -Write Disable(WRDI) instruction completion
	 * -Page Program(PP) instruction completion
	 * -Sector Erase(SE) instruction completion
	 * -Block Erase(BE) instruction completion
	 * -Chip Erase(CE) instruction completion
	 */

	/* Write Enable Command */
	flash_bit_bangout(CMD_WREN, SEND_COMMAND);

	flash_bit_bangout_done();
}

/* Flash Waiting READY ---------------------------------------------------- */
static int flash_wait_ready(void)
{
	/* 
	 * Program time: 1.4msec/page(typical,256-byte per page)
	 * Erase time  : 0.7sec/block(64KB per block)
	 */

	int ret=0, timecnt=0;
	u32 rd;

	/* Read Status Register(RDSR) Command */
	flash_bit_bangout(CMD_RD_STATUS, SEND_COMMAND);

	do {
		/* 3000000 is about 5 sec */
		if (timecnt++ > 3000000) {
			ret = -1;
			printk(KERN_ERR "MX25l: %s time out!!!", __FUNCTION__);
			break;
		}

		/* Status Register Out (8bit) */
		flash_bit_bangout(0x00, GET_DATA);

		/* Check WriteInProgress(WIP) bit */
		rd = ar7100_reg_rd(AR7100_SPI_RD_STATUS) & 0x1;

		if (timecnt % 30000 == 0) {
			/* Scheduling per 50msec */
			schedule();
		}
	} while (rd);

	flash_bit_bangout_done();

	return ret;
}

static int flash_sector_erase(u32 addr)
{
	int ret;

	/* Write Enable */
	flash_write_enable();

	/* Block Erase(BE) Command (Block=64KB) */
	flash_bit_bangout(CMD_BLOCK_ERASE, SEND_COMMAND);

	/* Destination Address */
	flash_bit_bangout(addr, SEND_ADDR);

	flash_bit_bangout_done();

	/* Wait Ready */
	ret = flash_wait_ready();

	return ret;
}

/**
 * Check dirty(written) a block
 *
 */
static int flash_is_dirty_block(F_BLOCK *blockp)
{
	int i;
	unsigned int *addr;
	unsigned int offset, len;

	if(blockp == F_END_FROM)
		return 0;

	/* Check whether there is a block to need to erase or not. */
	offset = blockp->offset;
	len   = blockp->size;
	addr = (unsigned int*)(MX25L_BASE_ADDR + offset);

	for(i=0; i<len; i+=sizeof(int), addr++) {
		/* Finish if address is over. */
		if(*addr != F_ERS_DAT)
			break;
	}
	if(i >= len)
		return 0;

    return 1;
}

/*
 * If more than 256bytes are sent to the device, the data of
 * the last 256-byte is programmed at the request page and
 * previous data will be disregarded.
 * If less than 256bytes are sent to the device, the data is
 * programmed at the request address of the page without
 * effect on other address of the same page. (by MX25L12805D)
 */
/*
 * If the eight least significant address bits(A7-A0) are not all 0,
 * all transmitted data which goes beyond the end of the current page
 * are programmed from the start address if(?) the same page (from the
 * address whose 8 least significant address bits(A7-A0) are all 0).
 * (by MX25L12805D)
 */
static int flash_bytes_write(u32 waddr, u8 *bufp, int length)
{
	int ret;
	int i;

	/* Write Enable */
	flash_write_enable();

	/* Page Program(PP) Command (Page=Max256Byte) */
	flash_bit_bangout(CMD_PAGE_PROG, SEND_COMMAND);

	/* Destination Address */
	flash_bit_bangout(waddr, SEND_ADDR);

	/* Write Data */
	for (i=0; i<length; i++)
		flash_bit_bangout(bufp[i], SEND_DATA);

	flash_bit_bangout_done();

	/* Wait Ready */
	ret = flash_wait_ready();

	return ret;
}


/* Erase (a sector of) flash memory --------------------------------------- */
int flash_mx25l_erase_sector(F_BLOCK **sectp)
{
	int i;
	int ret = 0;
	F_BLOCK **blockp;
	char *erase_flag;
	int block_num = 0;
	int need_erase = 0;

	blockp = sectp;
	while((*blockp) != F_END_FROM) {
		blockp++;
		block_num++;
	}

	erase_flag = (char*)kmalloc(block_num, GFP_KERNEL);
	if(erase_flag == NULL)
		return -ENOMEM;

	flash_mx25l_lock();

	/* Make dirty block list */
	for(i=0, blockp = sectp; i<block_num; i++, blockp++) {
		erase_flag[i] = flash_is_dirty_block(*blockp);
		if(erase_flag[i] == 1)
			need_erase = 1;
	}
	if(need_erase == 0) {
		/* Found no dirty blocks. Success! */
		ret = 0;
		goto end;
	}

	flash_func_select(1);

	/* Erase dirty blocks */
	for(i=0, blockp = sectp; i<block_num; i++, blockp++) {
		if(erase_flag[i] == 1) {
			ret = flash_sector_erase((*blockp)->offset);
			if(ret < 0) goto end;
		}
	}

	flash_func_select(0);

end:
	kfree(erase_flag);
	flash_mx25l_unlock();

	return (ret);
}

/* Erase (a block) on flash memory --------------------------------------- */
int flash_mx25l_erase_block(F_BLOCK *blockp)
{
	F_BLOCK *erase_sect[2];

	erase_sect[0] = blockp;
	erase_sect[1] = F_END_FROM;

	return flash_mx25l_erase_sector(erase_sect);
}


int flash_mx25l_read(F_BLOCK **blockp, unsigned long offset, unsigned int count, unsigned char *buf)
{
	unsigned long soffset, eoffset;	/* target block start logical offset, end logical offset */
	F_BLOCK **targetbp = blockp;
	unsigned char *bufp = buf;
	int rlen = count;
	int len;

	if (!rlen)
		return 0;

	/*
	 * Search the target block
	 */
	soffset = 0;
	eoffset = 0;
	while((*targetbp) != F_END_FROM) {
		eoffset = soffset + (*targetbp)->size;

		if(offset >= soffset &&  offset < eoffset)
			break;

		soffset = eoffset;
		targetbp++;
	}
	if((*targetbp) == F_END_FROM) {
		/* Not found target block */
		return -ENOSPC;
	}

	flash_mx25l_lock();

	if(offset != soffset) {
		/* Disallow read step over blocks */
		if(offset + rlen > eoffset)
			len = eoffset - offset;
		else
			len = rlen;

		memcpy(bufp, 
			(void*)(MX25L_BASE_ADDR + (*targetbp)->offset + (offset - soffset)),
			len);

		rlen -= len;
		bufp += len;
		targetbp++;

		if((*targetbp) == F_END_FROM) {
			flash_mx25l_unlock();
			return (bufp - buf);
		}
	}

	while(rlen > 0) {
		if(rlen > (*targetbp)->size)
			len = (*targetbp)->size;
		else
			len = rlen;

		memcpy(bufp, (void*)(MX25L_BASE_ADDR + (*targetbp)->offset), len);
		rlen -= len;
		bufp += len;
		targetbp++;

		if((*targetbp) == F_END_FROM) {
			flash_mx25l_unlock();
			return (bufp - buf);
		}
	}

	flash_mx25l_unlock();

	return count;
}

/*
 * Write memory block into flash memroy
 *
 */
static int flash_write_internal(u32 to, u8 *from, unsigned long len)
{
	int ret = 0;
	int rest_len, writable_len, write_len;

	if (len == 0)
		return (0);

	/* Function Select->1 */
	flash_func_select(1);

	rest_len = len;
	while (rest_len > 0) {
		/* Caluculate Writing data length.
		 * (1) Writing flash ROM offset must be 256 alignment.
		 * (2) Writing data length must be less than 256 Byte.
		 */
		writable_len = 256 - (to & 0xff);
		write_len = (rest_len > writable_len) ? writable_len : rest_len;

		/* Write FLASH (Max data length 256Byte in 1 time) */
		ret = flash_bytes_write(to, from, write_len);
		if (ret != 0)
			break;

		to += write_len;
		from += write_len;
		rest_len -= write_len;
	}

	/* Function Select->0 */
	flash_func_select(0);

	return (ret);
}

int flash_mx25l_write(F_BLOCK **blockp, unsigned long offset, unsigned int count, unsigned char *buf)
{
	unsigned long soffset, eoffset;	/* target block start logical offset, end logical offset */
	F_BLOCK **targetbp = blockp;
	unsigned char *bufp = buf;
	int ret, wlen = count, len;

	if (!wlen)
		return 0;

	/*
	 * Search the target block
	 */
	soffset = 0;
	eoffset = 0;
	while((*targetbp) != F_END_FROM) {
		eoffset = soffset + (*targetbp)->size;

		if(offset >= soffset &&  offset < eoffset)
			break;

		soffset = eoffset;
		targetbp++;
	}
	if((*targetbp) == F_END_FROM) {
		/* Not found target block */
		return -ENOSPC;
	}

	flash_mx25l_lock();

	if(offset != soffset) {
		/* Disallow read step over blocks */
		if(offset + wlen > eoffset)
			len = eoffset - offset;
		else
			len = wlen;

		ret = flash_write_internal((*targetbp)->offset + offset - soffset, bufp, len);
		if(ret != 0) {
			flash_mx25l_unlock();
			return -1;
		}

		wlen -= len;
		bufp += len;
		targetbp++;
	}

	if((*targetbp) == F_END_FROM) {
		/* Not found target block */
		flash_mx25l_unlock();
		return (bufp - buf);
	}

	while(wlen > 0) {
		if(wlen > (*targetbp)->size)
			len = (*targetbp)->size;
		else
			len = wlen;

		ret = flash_write_internal((*targetbp)->offset, bufp, len);
		if(ret != 0) {
			flash_mx25l_unlock();
			return -1;
		}

		wlen -= len;
		bufp += len;
		targetbp++;

		if((*targetbp) == F_END_FROM) {
			/* Not found target block */
			flash_mx25l_unlock();
			return (bufp - buf);
		}
	}

	flash_mx25l_unlock();

	return count;
}

/* Initalize Flash ROM ----------------------------------------------------- */
int flash_drv_init(struct sxflash_operations *flashop)
{
	printk(KERN_INFO "MX25l: Serial Flash ROM Driver init \n");

	init_MUTEX(&flash_sem);

	/* Function Select->1 */
	flash_func_select(1);

	/* Remap Enable(Maintain), CLOCK DIVIDER = AHB CLOCK/(2<<4) */
	/* This setting do in the cream loader. */
	/* ar7100_reg_wr_nf(AR7100_SPI_CLOCK, 0x42); */

	/* CS_0=1(Dis) */
	ar7100_reg_wr_nf(AR7100_SPI_WRITE, 0x00070000);

	/* Function Select->0 */
	flash_func_select(0);

	flashop->lock = flash_mx25l_lock;
	flashop->unlock = flash_mx25l_unlock;
	flashop->erase_sect = flash_mx25l_erase_sector;
	flashop->erase_block = flash_mx25l_erase_block;
	flashop->read = flash_mx25l_read;
	flashop->write = flash_mx25l_write;

	return (0);
}
