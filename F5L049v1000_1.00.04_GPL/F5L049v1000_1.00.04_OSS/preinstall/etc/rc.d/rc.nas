#!/bin/sh

CTRLDATA="/etc/sysconfig/system.conf"
SXNASCTRL="/usr/sbin/sxnasctrl"

if [ ! -f ${CTRLDATA} ]; then
	echo "Error:Not found ${CTRLDATA}"
	exit 1
fi
if [ ! -x ${SXNASCTRL} ]; then
	echo "Error:Not found ${SXNASCTRL}"
	exit 1
fi

# samba and vsftp start
${SXNASCTRL} -c ${CTRLDATA}

exit 0
