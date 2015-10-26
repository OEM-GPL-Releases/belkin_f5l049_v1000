#!/bin/sh

# DLNA
DLNAD=/usr/bin/ushare
# SAMBA
SMBCONF=/usr/sbin/sxsambaconf
# LED
LEDPATH="/proc/silex/led"

# test code
#DMSG="${ACTION}:${SHAREPATH}:${DEVPATH}"
#echo "hotplug.misc ${DMSG}" | logger -p 4

dlna() {
	if [ ! -x ${DLNAD} ]; then
		return 1
	fi

	killall -15 ushare
	sleep 3
	killall -9 ushare

	if [ "$ACTION" = "BEFOREMNT" ]; then
		return 0
	fi

	local mount=`sxstorageinfo -s`
	local mname=`sxromconf -c GET_MACHINE`
	local dir=`sxsysconf DLNA_STORAGE`
	local port=`sxsysconf DLNA_PORT_NUMBER`
	local hname=`sxsysconf HOST_NAME`
	local sirial=`sxromconf -c GET_SERIALNUM`

	if [ -z "$dir" ]; then
		dir="ALL"
	fi

	if [ -z "$sirial" ]; then
		sirial=`sxsysconf -f /etc/silex/romver.conf SERIALNUMBER`
	fi

	if [ "${mount}" = "mounted" ]; then
		${DLNAD} -D -m "${mname}" -p "${port}" -s "${sirial}" -c "${dir}" -n "${hname}"
	fi
	return 0
}

usb_led() {
	LED="${LEDPATH}/$1"
	PORT="1.1.$1"
	MOUNTED=$2
	UNMOUNT=$3
	if [ ! -d "/sys/bus/usb/devices/1-1.$1" ] ; then
		echo "0 0 1 3 0 0 0" > "$LED"
	fi
	TMP=`sxstorageinfo -p $PORT`
	if [ $? != 0 ]; then
		return 0
	fi

	if [ "$TMP" = "mounted" ]; then
		echo "$MOUNTED" > "$LED"
	elif [ "$TMP" = "umount" ]; then
		echo "$UNMOUNT" > "$LED"
	elif [ "$TMP" = "not support" ]; then
		echo "$UNMOUNT" > "$LED"
	fi
}


case "${ACTION}" in
"BEFOREMNT" )
	usb_led "1" "200 2 2 3 0 0 0" "200 2 2 3 0 0 0"
	usb_led "2" "200 2 2 3 0 0 0" "200 2 2 3 0 0 0"
	usb_led "3" "200 2 2 3 0 0 0" "200 2 2 3 0 0 0"
	usb_led "4" "200 2 2 3 0 0 0" "200 2 2 3 0 0 0"
	;;
"AFTERMNT" )
	$SMBCONF -c "${DEVPATH}/smb.dir.conf" -d "/etc/samba/smb.def.conf" 
	usb_led "1" "0 1 1 3 0 0 0" "0 0 1 3 0 0 0"
	usb_led "2" "0 1 1 3 0 0 0" "0 0 1 3 0 0 0"
	usb_led "3" "0 1 1 3 0 0 0" "0 0 1 3 0 0 0"
	usb_led "4" "0 1 1 3 0 0 0" "0 0 1 3 0 0 0"
	;;
esac

dlna

