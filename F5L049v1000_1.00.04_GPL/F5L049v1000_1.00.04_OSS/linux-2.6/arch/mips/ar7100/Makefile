#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
#

# Makefile for Atheros ar531x boards
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

#.S.s:
#	$(CPP) $(CFLAGS) $< -o $*.s
#.S.o:
#	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $*.o

obj-y	+=	prom.o	\
			irq.o	\
			setup.o	\
			platform.o \

obj-$(CONFIG_AG7100_PROCFS) += ag7100_proc.o

obj-$(CONFIG_SXFLASH)   +=  flashrom/

obj-$(CONFIG_AR9100)	+= wdt.o

# silex standard modules
obj-$(CONFIG_SXLEDCTL)  += led_ctrl.o
obj-$(CONFIG_SXLEDPROC) += led_proc.o
obj-$(CONFIG_SXSWCTL)   += sw_ctrl.o
obj-$(CONFIG_SXSWPROC)  += sw_proc.o
obj-$(CONFIG_SXFLASH)   += flashrom/
