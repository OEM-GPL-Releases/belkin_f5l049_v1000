#!/bin/sh

WPS_SW=1
RESET_SW=2
ACTION_UP=0
ACTION_DOWN=1

SEC=0

UMOUNTPATH="/mnt/shared"
LEDPATH="/proc/silex/led"
# -------------------------------------------------------------------
WL_WEP_KEYLEN="`sxsysconf WL_WEP_KEYLEN`"
WL_WEP_KEY0="`sxsysconf WL_WEP_KEY0`"
WL_WEP_KEY1="`sxsysconf WL_WEP_KEY1`"
WL_WEP_KEY2="`sxsysconf WL_WEP_KEY2`"
WL_WEP_KEY3="`sxsysconf WL_WEP_KEY3`"
WL_CHANNEL="`sxsysconf WL_CHANNEL`"
WL_NETWORK_AUTH="`sxsysconf WL_NETWORK_AUTH`"
WL_WEP_KEY_ID="`sxsysconf WL_WEP_KEY_ID`"
WL_MODE="`sxsysconf WL_MODE`"
WL_WPA_ENCRIPT="`sxsysconf WL_WPA_ENCRIPT`"
WL_WEP_ENABLE="`sxsysconf WL_WEP_ENABLE`"
WL_SSID="`sxsysconf WL_SSID`"
WL_WPA_PSK="`sxsysconf WL_WPA_PSK`"
WL_WPS_ENABLE="`sxsysconf WL_WPS_ENABLE`"
WL_WPS_PIN="`sxsysconf WL_WPS_PIN`"

debug() {
	echo "$0:$SW: $1 (${SEC} sec)" | logger -p 4 
}

# WPS button action !!
wps_action() {
	local PBCPID=
	local PINPID=
	local WPS_PBC_SEC=3
	local WPS_PIN_SEC=10

	if [ ${ACTION} -eq ${ACTION_DOWN} ]; then
		debug "ACTION_DOWN"
		touch /tmp/wps_doing
		/usr/sbin/led_wps_pbc.sh ${WPS_PBC_SEC} >/dev/null &
		/usr/sbin/led_wps_pin.sh ${WPS_PIN_SEC} >/dev/null &
		retuen 0
	elif [ ${ACTION} -eq ${ACTION_UP} ]; then
		debug "ACTION_UP"
		# rm /tmp/wps_doing , after then make by wpa_supplicant
		rm /tmp/wps_doing
		PBCPID=`cat /var/run/led_wps_pbc.pid`
		PINPID=`cat /var/run/led_wps_pin.pid`
		kill ${PBCPID} 2>/dev/null
		kill ${PINPID} 2>/dev/null
	
		# Push 0 to 3sec : LED off
		# InProgress LED control by wps_suplicant
		if [ ${SEC} -ge ${WPS_PBC_SEC} ] && [ ${SEC} -lt ${WPS_PIN_SEC} ]; then
			debug "LED off"
			ps www | grep security_led.sh | grep -v grep | awk '{print $1}' | xargs kill -9
			echo "0 0 1 3 0 0 0" > /proc/silex/led/6
			debug "WPS PBC"
			/usr/sbin/wps_pbc >/dev/null &
		elif [ ${SEC} -ge ${WPS_PIN_SEC} ]; then
			debug "LED off"
			ps www | grep security_led.sh | grep -v grep | awk '{print $1}' | xargs kill -9
			echo "0 0 1 3 0 0 0" > /proc/silex/led/6
			debug "WPS PIN"
			/usr/sbin/wps_pin >/dev/null &
		fi
		return 0
	fi
}

# USB LED on/off
usb_led() {
	local VAL1=$1
	local VAL2=$2
	local I=
	local LED=
	local TMP1=
	local TMP2=

	for I in `ls /sys/bus/usb/devices/`
	do
		TMP1=`echo ${I} | cut -c 1-4`
		TMP2=`echo ${#I}`
		if [ -z $TMP1 -o $TMP2 -ne 5 ]; then
			continue
		fi
		if [ "$TMP1" != "1-1." ]; then
			continue
		fi

		LED=`echo ${I} | cut -c 5`
		if [ $LED -gt 4 -o $LED -lt 1 ]; then
			continue
		fi
		TMP1=`sxstorageinfo -p $I`
		if [ "$TMP1" = "mounted" ]; then
			echo $VAL1 > "${LEDPATH}/${LED}"
		elif [ "$TMP1" = "umount" ]; then
			echo $VAL2 > "${LEDPATH}/${LED}"
		elif [ "$TMP1" = "not support" ]; then
			echo "0 0 1 3 0 0 0" > "${LEDPATH}/${LED}"
		fi
	done
}

force_umount() {
	/etc/rc.d/rc.samba  stop
	/etc/rc.d/rc.vsftpd stop
	/usr/sbin/picshare_killall_job.sh
	sxmount umount &
	/etc/rc.d/rc.samba  start
	/etc/rc.d/rc.vsftpd start
}

# Reset button action !!
reset_action() {
	local TMP=`sxstorageinfo -S`
	if [ $ACTION -eq $ACTION_DOWN ]; then
		if [ "$TMP" = "umount" ]; then
			usb_led "0 1 1 3 0 0 0" "3000 0 1 1 3000 1 0"
		elif [ "$TMP" = "mounted" ]; then
			usb_led "3000 1 1 1 3000 0 0" "0 0 1 3 0 0 0"
		fi
		return 0
	elif [ $ACTION -ne $ACTION_UP ]; then
		return 1
	fi

	if [ $SEC -lt 3 ] ; then
		usb_led "0 1 1 3 0 0 0" "0 0 1 3 0 0 0"
		return 0
	fi

	if [ "$TMP" = "umount" ]; then
		usb_led "0 1 1 3 0 0 0" "200 2 2 3 0 0 0"
		sxmount mount &
	elif [ "$TMP" = "mounted" ]; then
		usb_led "200 2 2 3 0 0 0" "0 0 1 3 0 0 0"
		force_umount &
	fi
	return 0
}


# Start script
SEC=`expr $TIME / 1000`

case $SW in
$WPS_SW )
	# WPS Support only Wireless mode.
	if [ "${WL_WPS_ENABLE}" != "ENABLE" ]; then
		debug "WPS Disable: WPS is ${WL_WPS_ENABLE}"
		exit 1
	fi
	if [ "${WL_MODE}" = "Ad Hoc" ]; then
		debug "WPS Disable: Wireless mode is ${WL_MODE}"
		exit 1
	fi
	# if until eth0 device is ready
	if ! ifconfig -a | grep eth0 > /dev/null ; then
		debug "WPS Disable: device is not ready"
		exit 1
	fi
	# is Wireless only
	if ifconfig -a | grep ath0 > /dev/null ; then
		debug "WPS Disable: WPS is Wireless only"
		exit 1
	fi
	debug "WPS Action"
	wps_action
	;;
$RESET_SW )
	reset_action
	;;
*)
  ;;
esac

