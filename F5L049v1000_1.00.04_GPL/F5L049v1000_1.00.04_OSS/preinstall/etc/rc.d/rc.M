#!/bin/sh

SERVICES='\
	rc.syslog \
	rc.swintr \
	rc.nic \
	rc.network \
	rc.sntp \
	rc.hotplug \
	rc.inetd \
	rc.thttpd \
	rc.mdns \
	rc.jcpd \
	rc.nas \
	rc.sxuptp \
	rc.usbstorage \
	rc.picshare \
	rc.niclink \
	rc.led \
	'

cd /etc/rc.d

for SERV  in ${SERVICES}
do
	if [ -x ./${SERV} ]; then
		./${SERV} start
	fi
done
