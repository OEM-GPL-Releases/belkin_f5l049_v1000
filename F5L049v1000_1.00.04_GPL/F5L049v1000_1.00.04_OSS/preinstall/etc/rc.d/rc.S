#!/bin/sh

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
export PATH

SYSCONF_DIR='/etc/sysconfig'
ROMVER_CONF='/etc/silex/romver.conf'

# This is used to force the Linux VM to keep a 
# minimum number of kilobytes free.
# See linux-2.6/Documentation/sysctl/vm.txt
MIN_FREE_KBYTES=8192

NEED_ROM_WRITE=0

ArrangeSystem(){
	mkdir -m 777 /tmpfs/tmp
	mkdir -m 755 /tmpfs/tmp/picshare
	mkdir -m 755 /tmpfs/dev
	mkdir -m 555 /tmpfs/dev/ath
	mkdir -m 755 /tmpfs/mnt
	mkdir -m 755 /tmpfs/mnt/shared
	mkdir -m 755 /tmpfs/var
	mkdir -m 755 /tmpfs/var/lib
	mkdir -m 755 /tmpfs/var/cache
	mkdir -m 777 /tmpfs/var/lock
	mkdir -m 755 /tmpfs/var/lock/picshare
	mkdir -m 755 /tmpfs/var/log
	mkdir -m 755 /tmpfs/var/run
	mkdir -m 755 /tmpfs/var/spool
	mkdir -m 755 /tmpfs/etc
	mkdir -m 755 /tmpfs/etc/dhcpc
	mkdir -m 755 /tmpfs/etc/sysconfig

	cp /etc/passwd.org /tmpfs/etc/passwd
	cp /etc/group.org  /tmpfs/etc/group
	cp /etc/shadow.org /tmpfs/etc/shadow
	cp /etc/fstab.org  /tmpfs/etc/fstab
	touch /tmpfs/etc/mtab
	touch /tmpfs/etc/vsftpd.conf
}

ConfigSystem() {
	echo -n "RANDOM init : "
	sxrandinit
	if [ $? -eq 0 ] ; then
		echo "Success"
	else
		echo "Fail"
	fi

	echo ${MIN_FREE_KBYTES} > /proc/sys/vm/min_free_kbytes
}

Mount(){
	echo 'Mounting proc filesystem'
        mount -v proc /proc -n -t proc
        echo 'Mounting sysfs filesystem'
        mount -v sysfs /sys -n -t sysfs
	echo 'Mounting usbfs filesystem'
	mount -t usbfs none /proc/bus/usb
	echo 'Mounting tmpfs filesystem'
	mount -t tmpfs -o size=1m tmpfs /tmpfs
	echo 'Mounting /etc/fstab'
	mount -v -a -t nonfs,nosmbfs,noproc
}


RstoreTime(){
	echo "Set H/W clock:                          hwclock -l -s"
	hwclock -l -s
	if [ $? -ne 0 ] ; then
		echo "Error: hwclock"
	fi
}

SetHostname(){
	HOSTNAME=`sxsysconf HOST_NAME`
	if [ "${HOSTNAME}" = "" ]; then
		# Default setting.
		hostname "sxlinux"
	else
		hostname "${HOSTNAME}"
	fi
}

SetLocaltime(){
	TIMEZONE=`sxsysconf TIMEZONE`
	ZONE=`echo "${TIMEZONE}" | sed -e 's/^\([+-]\)0\?\([0-9]\+\?\):.*/\1\2/'`
	echo -n "localtime "

	case "${ZONE}" in
		'+12' | '+11' | '+10' | '+9' ) GMTNAME=GMT${ZONE} ;;
		 '+8' |  '+7' |  '+6' | '+5' ) GMTNAME=GMT${ZONE} ;;
		 '+4' |  '+3' |  '+2' | '+1' ) GMTNAME=GMT${ZONE} ;;
		 '-4' |  '-3' |  '-2' | '-1' ) GMTNAME=GMT${ZONE} ;;
		 '-8' |  '-7' |  '-6' | '-5' ) GMTNAME=GMT${ZONE} ;;
		'-12' | '-11' | '-10' | '-9' ) GMTNAME=GMT${ZONE} ;;
		                           * ) GMTNAME=GMT ;;
	esac

	ln -s /usr/share/zoneinfo/Etc/${GMTNAME} /tmpfs/etc/localtime
	echo "${TIMEZONE}"
	
	# default date
	date 2009.01.01-00:00:00
}

InitDefaultConfig(){
	# Write Default setting data to $SYSCONF_DIR.
	# You can modified freely :)

	cp /etc/silex/defsystem.conf ${SYSCONF_DIR}/system.conf
	cp /etc/silex/pict* ${SYSCONF_DIR}/
	DEFAULT_HOSTNAME=`sxmkhostname BK-HB`
	sxsysconf -w ${DEFAULT_HOSTNAME} HOST_NAME
	if [ $? -ne 0 ]; then
		echo "Error: Could not save HOSTNAME"
	fi

	DEFAULT_WPSPIN=`sxwpspin`
	sxsysconf -w ${DEFAULT_WPSPIN} WL_WPS_PIN
	if [ $? -ne 0 ]; then
		echo "Error: Could not save WL_WPS_PIN"
	fi

	sxromconf -c STORE_CFG -o ${SYSCONF_DIR}
	NEED_ROM_WRITE=1
}

Loadsysconfig(){
	local LEN

	LEN=`sxromconf -c GET_CONFLEN`
	if [ "${LEN}" = "0" ]; then
		echo "Factory default setting detected!"
		InitDefaultConfig
	else
		sxromconf -c LOAD_CFG -o ${SYSCONF_DIR}
	fi
}

# =======================================================
# =  Main initial start                                 =
# =======================================================
#RstoreTime
Mount
ConfigSystem
ArrangeSystem
Loadsysconfig
SetLocaltime
SetHostname

if [ "${NEED_ROM_WRITE}" = "1" ]; then
	sxromconf -c FLUSHCACHE
fi

# Multi User mode 
if [ -x /etc/rc.d/rc.M ]; then
  . /etc/rc.d/rc.M
fi

# serial login
DEBUGMODE=`sxromconf -c GET_DEBUG`
if [ "${DEBUGMODE}" = "on" ]; then
  /sbin/getty -L ttyS0 115200 vt100 &
else
  echo "Serial login always is disabled by DEBUGMODE setting!"
fi

