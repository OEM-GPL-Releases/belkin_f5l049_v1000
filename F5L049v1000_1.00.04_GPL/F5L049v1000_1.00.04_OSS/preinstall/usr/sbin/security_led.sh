#!/bin/sh

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
	echo "$0:$SW: $1" | logger -p 5
}

# file
IF_NAME="eth0"

ctrl_security_led() {

	SECURITY_LED=0;

	case "${WL_MODE}" in
	'Infrastructure') 
	NOTASSOC=`iwconfig ${IF_NAME} | grep -F 'Access Point: Not-Associated' | wc -l`;;
	'Ad Hoc')
	NOTASSOC=`iwconfig ${IF_NAME} | grep -F 'Cell: Not-Associated' | wc -l`;;
	esac
		
#	ENCOFF=`iwconfig ${IF_NAME} | grep -F 'Encryption key:off' | wc -l`;

	# Associated
	if [ "${NOTASSOC}" != "1" ]; then
		debug "iwconfig = Associated"
		# Encryption key is on
#		if [ "${ENCOFF}" != "1" ]; then
#			debug "iwconfig = Encryption key is on"
			SECURITY_LED=1;
#		fi
	fi

	if [ "${SECURITY_LED}" = "1" ]; then
		debug "LED on";
		echo "0 1 1 3 0 0 0" > /proc/silex/led/6;
	else
		debug "LED off";
		echo "0 0 1 3 0 0 0" > /proc/silex/led/6;
	fi	
}	

# BEGIN
# only Wireless mode.

# if until eth0 device is ready
while ! ifconfig -a | grep eth0 > /dev/null
do
	sleep 1;
	debug "device is not ready -> wait"
done

# is Wireless only
if ifconfig -a | grep ath0 > /dev/null ; then
	debug "Wireless only -> exit"
	exit 1;
fi

# -- main --
debug "Start Security LED Process"i;

WPS_DOING=0;

while :
do
	# 
	sleep 1;

	# check process WPS, check swintr.sh pushing
	# WPS_DOING = `ps www | grep wpa_supplicant | egrep '(Mpin\|Mpbc)' | grep -v grep | wc -l`
	if [ -f /tmp/wps_doing ]; then
		debug "WPS doing"
		WPS_DOING=1;
		continue;
	fi

	# if WPS done, then waiting ERROR LED Process
	if [ "${WPS_DOING}" = "1" ]; then
		debug "WPS done -> wait 15 sec for EROOR LED Process"
		sleep 15;
		WPS_DOING=0;
	fi

	# is WEP or WPS
	case "${WL_NETWORK_AUTH}" in
		'Open'|'Shared')
		if [ "${WL_WEP_ENABLE}" = "ON" ]; then
			debug "WEP mode"
			# if set WEP but not store key
			NOKEY=`iwlist ${IF_NAME} keys | grep -F 'Current Transmit Key: [0]' | wc -l`;
			if [ "${NOKEY}" != "1" ]; then
				debug "WEP mode Key is set"
				ctrl_security_led
			fi
		fi
		;;
		'WPA'|'WPA2')
		debug "WPA mode"
		ctrl_security_led	
		;;
	esac

done

