#
# Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2005 Atheros Communications, Inc.
# All rights reserved.
#
DEPTH=	../../..

ATHDIR=	${DEPTH}/common/ath_dev
HALDIR=	${DEPTH}/common/hal
LINUX_HALDIR=	${DEPTH}/common/hal/linux
LINUX_INCLUDE=	${DEPTH}/include/linux

CC = $(CROSS_COMPILE)gcc

CFLAGS=	-I. -I${LINUX_INCLUDE} -I${HALDIR} -I${LINUX_HALDIR} -I${ATHDIR} -g

ALL=	eeprom rfgain ani txpow reg

all:	${ALL}

eeprom: eeprom.o
	${CC} -o eeprom ${LDFLAGS} $^
rfgain: rfgain.o diag.o
	${CC} -o rfgain ${LDFLAGS} $^
ani:    ani.o diag.o
	${CC} -o ani ${LDFLAGS} $^
txpow:  txpow.o
	${CC} -o txpow ${LDFLAGS} $^
reg:	reg.o
	${CC} -o reg ${LDFLAGS} $^

install: ${ALL}
	for i in ${ALL}; do \
		rm -f /usr/local/bin/$$i; \
		cp $$i /usr/local/bin/$$i; \
	done

clean:
	rm -f ${ALL} core

.c.o:
	$(CC) -c $< ${CFLAGS}
