#!/bin/sh

# operation state
# up / down / dormant / unknown / ?...

# file
IF_NAME="eth0"
INTERFACE="/sys/class/net/${IF_NAME}"
NETWORK="/etc/rc.d/rc.network"
SXNOTIFY="/usr/bin/sxnotify"

# config
DT_WIRELESS=5
DT_WIRED=5
DEF_WAITTIME=12

get_ipaddr() {
	if ! /sbin/ifconfig ${IFNAME} | grep "inet addr" 1> /dev/null ; then
		IP_ADDR="0.0.0.0"
	else
		IP_ADDR=`/sbin/ifconfig ${IFNAME} | sed -n '/.*inet addr:\([0-9.]*\).*/{s//\1/;p;q}'`
	fi
}

TMP=`date -u | cut -c 11-19`
HOUR=`echo ${TMP} | cut -d ":" -f 1`
MIN=`echo ${TMP} | cut -d ":" -f 2`
SEC=`echo ${TMP} | cut -d ":" -f 3`
TIME=`expr ${HOUR} \* 3600 + ${MIN} \* 60 + ${SEC}`
REV_TIME=${TIME}
MAXDT=

while :
do
	${SXNOTIFY} -f ${INTERFACE}/operstate -p -i 1000
	if [ $? -ne 0 ]; then
		continue
	fi

	if [ -f /tmp/wps_doing ]; then
		continue
	fi

	LINK=`cat ${INTERFACE}/operstate`
	TMP=`date -u | cut -c 11-19`
	HOUR=`echo ${TMP} | cut -d ":" -f 1`
	MIN=`echo ${TMP} | cut -d ":" -f 2`
	SEC=`echo ${TMP} | cut -d ":" -f 3`
	TIME=`expr ${HOUR} \* 3600 + ${MIN} \* 60 + ${SEC}`
	if [ ${LINK} = up ]; then
		DT=`expr ${TIME} - ${REV_TIME}`
		if [ -d ${INTERFACE}/wireless ]; then
			MAXDT=${DT_WIRELESS}
		else
			MAXDT=${DT_WIRED}
		fi
		if [ ${DT} -lt 0 -o ${DT} -ge ${MAXDT} ]; then
			get_ipaddr
			IP_ADDR_REV=${IP_ADDR}
			${NETWORK} restart ${IF_NAME} ${DEF_WAITTIME}
			get_ipaddr
			if [ ${IP_ADDR} != ${IP_ADDR_REV} ]; then
				/usr/sbin/ushare_console restart 2>/dev/null
			fi
		fi
	fi
	REV_TIME=${TIME}
done

