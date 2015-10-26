#!/bin/sh

ACTION=$1
DIRTY=$2

case $ACTION in
"start" )
	if [ $DIRTY = 1 ]; then
		/etc/rc.d/rc.samba  start
		/etc/rc.d/rc.vsftpd start
	fi
	;;
"stop" )
	if [ $DIRTY = 1 ]; then
		/etc/rc.d/rc.samba  stop
		/etc/rc.d/rc.vsftpd stop
	fi
	;;
esac
