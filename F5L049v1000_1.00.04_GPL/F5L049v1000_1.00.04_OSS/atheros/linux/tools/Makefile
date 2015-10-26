# $Id: //depot/sw/releases/7.1_AP/wlan/linux/tools/Makefile#3 $
#
# @@@mod@@@
#
# Makefile for Atheros tools.
#

ifeq ($(obj),)
obj := .
endif

#
# @@@ Specified COPT - silex @@@
CC=$(CROSS_COMPILE)gcc

DEPTH := ../..

BINDIR=	/usr/local/bin

include $(obj)/${DEPTH}/linux/Makefile.inc
include ./../../linux/Makefile.inc


ALL=	athstats athstatsclr 80211stats athkey wlanconfig \
	pktlogconf pktlogdump radartool
# athchans athctrl athdebug 80211debug athcwm athampdutrc atrc

all:	${ALL}

INCS+=	-I${HAL} -I${HAL}/${OS} -I${ATH_HAL} -I${obj}/${DEPTH} -I${COMMON} -I${ATH_RATE} -I${ATH} -I${WLAN}
#CFLAGS=	${INCS} -g -static

# @@@ Specified COPT - silex @@@
COPTS= -g -Werror -O2  -Wno-undef  -DAH_BYTE_ORDER=AH_BIG_ENDIAN -D=AH_REGOPS_FUNC -D__LINUX_MIPS32_ARCH__ -mlong-calls -isystem -DATH_CWM=1 -DATH_CHAINMASK_SELECT=1 -DATH_FORCE_BIAS=1 -DATH_SUPPORT_UAPSD=1

CFLAGS=	${INCS} ${COPTS}
#ifeq (${BUILD_STATIC}, y)
#LDFLAGS= -static
#endif

all:	${ALL}

ath_tools: athstats athstatsclr pktlogconf pktlogdump radartool 80211stats
	$(STRIP) athstats athstatsclr pktlogconf pktlogdump radartool 80211stats;\
	cp athstats $(INSTALL_ROOT)/sbin;\
	cp athstatsclr $(INSTALL_ROOT)/sbin;\
	cp pktlogconf $(INSTALL_ROOT)/sbin;\
	cp radartool $(INSTALL_ROOT)/sbin;\
	cp 80211stats $(INSTALL_ROOT)/sbin;\
	cp pktlogdump $(INSTALL_ROOT)/sbin;

#- athampdutrc athcwm atrc pktlogconf pktlogdump radartool 

athstats: athstats.o
	${CC} -o athstats ${LDFLAGS} $^

athstatsclr: athstatsclr.o
	${CC} -o athstatsclr ${LDFLAGS} $^

athctrl: athctrl.o
	${CC} -o athctrl ${LDFLAGS} $^

athampdutrc: athampdutrc.o
	${CC} -o athampdutrc ${LDFLAGS} $^

80211stats: 80211stats.o
	${CC} -o 80211stats ${LDFLAGS} $^
athkey: athkey.o
	${CC} -o athkey ${LDFLAGS} $^


athdebug: athdebug.o
	${CC} -o athdebug ${LDFLAGS} $^
wlanconfig: wlanconfig.o
	${CC} -o wlanconfig ${LDFLAGS} $^
80211debug: 80211debug.o
	${CC} -o 80211debug ${LDFLAGS} $^
athcwm: athcwm.o
	${CC} -o athcwm ${LDFLAGS} $^
atrc: atrc.o
	${CC} -o atrc ${LDFLAGS} $^
pktlogdump: pktlogdump.o
	${CC} -o pktlogdump ${LDFLAGS} $^ -lm
pktlogconf: pktlogconf.o
	${CC} -o pktlogconf ${LDFLAGS} $^
radartool: radartool.o
	${CC} -o radartool ${LDFLAGS} $^

install: ${ALL}
	cp athstats ${DESTDIR}${BINDIR}
	cp athstatsclr ${DESTDIR}${BINDIR}
	cp athampdutrc ${DESTDIR}${BINDIR}
	cp athcwm ${DESTDIR}${BINDIR}
	cp athdebug ${DESTDIR}${BINDIR}
	cp 80211stats ${DESTDIR}${BINDIR}
	cp atrc ${DESTDIR}${BINDIR}
	cp pktlogconf ${DESTDIR}${BINDIR}
	cp pktlogdump ${DESTDIR}${BINDIR}
	-rm ${DESTDIR}/${BINDIR}/wlanstats
	ln ${DESTDIR}/${BINDIR}/80211stats ${DESTDIR}/${BINDIR}/wlanstats
	cp 80211debug ${DESTDIR}${BINDIR}
	-rm ${DESTDIR}/${BINDIR}/wlandebug
	ln ${DESTDIR}/${BINDIR}/80211debug ${DESTDIR}/${BINDIR}/wlandebug
	cp wlanconfig ${DESTDIR}${BINDIR}
	cp radartool ${DESTDIR}${BINDIR}/radartool

ath_tools_clean:
	@rm -f athstats athstatsclr pktlogdump pktlogconf radartool

clean: ath_tools_clean
	rm -f ${ALL} core a.out

.c.o:
	$(CC) -c $< ${CFLAGS} -I../ath -I../atrc -I$(COMMON)/ath_pktlog -I$(COMMON)/dfs
