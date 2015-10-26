#!/bin/sh

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
export PATH

if [ -x /etc/rc.d/rc.picshare ]; then
  . /etc/rc.d/rc.picshare stop
fi

if [ -x /etc/rc.d/rc.mdns ]; then
  . /etc/rc.d/rc.mdns stop
fi

if [ -x /etc/rc.d/rc.jcpd ]; then
  . /etc/rc.d/rc.jcpd stop
fi

if [ -x /etc/rc.d/rc.crond ]; then
  . /etc/rc.d/rc.crond stop
fi

if [ -x /etc/rc.d/rc.sntp ]; then
  . /etc/rc.d/rc.sntp stop
fi

if [ -x /etc/rc.d/rc.samba ]; then
  . /etc/rc.d/rc.samba stop
fi

if [ -x /etc/rc.d/rc.vsftpd ]; then
  . /etc/rc.d/rc.vsftpd stop
fi

if [ -x /etc/rc.d/rc.hotplug ]; then
  . /etc/rc.d/rc.hotplug stop
fi

if [ -x /etc/rc.d/rc.sxuptp ]; then
  . /etc/rc.d/rc.sxuptp stop
fi
