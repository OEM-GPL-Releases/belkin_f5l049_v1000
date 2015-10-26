#!/bin/bash
#
# install.sh : Create compressed rootfs image
#
# Copyright (C) 2006 - 2008 silex technology, Inc.  All rights reserved.

#############################################
# Configuration development environment     #
#############################################
PACKAGE_LIST=$1
KERNEL_PATH=$2
SYSROOT_PATH=$3

PREINSTALL_DIR="../preinstall"
TMPDIR="./tmpdir"
ROOTFS_FNAME="rootfs"
STRIP="${CROSS_COMPILE}strip"

install_bin() {
	local SRCBIN=$1
	local INSTALLPATH=$2

	if [ -f ${SRCBIN} ]; then
		cp ${SRCBIN} ${INSTALLPATH}
		if [ $? != 0 ]; then
			exit 1
		fi
		${STRIP} ${INSTALLPATH}
		if [ $? != 0 ]; then
			exit 1
		fi
	else
		echo "Not found ${SRCBIN}"
		exit 1
	fi
}

install_lib() {
	local SRCBIN=$1
	local INSTALLPATH=$2

	if [ -f ${SRCBIN} ]; then
		cp ${SRCBIN} ${INSTALLPATH}
		if [ $? != 0 ]; then
			exit 1
		fi
		${STRIP} --strip-unneeded ${INSTALLPATH}
		if [ $? != 0 ]; then
			exit 1
		fi
	else
		echo "Not found ${SRCBIN}"
		exit 1
	fi
}

install_file() {
	local SRCBIN=$1
	local INSTALLPATH=$2

	cp -rf ${SRCBIN} ${INSTALLPATH}
	if [ $? != 0 ]; then
		exit 1
	fi
}

kernel() {
	local MODULES
	local MOD

	if [ ! -d ${KERNEL_PATH} ]; then
		echo "Not found kernel ${KERNEL_PATH}"
		exit 1
	fi

	# find the kernel module, and copy rootfs
	MODULES=`find ${KERNEL_PATH}/ -name '*.ko' -type f`
	for MOD in ${MODULES}
	do
		cp -rp ${MOD} ${TMPDIR}/lib/modules
		if [ $? != 0 ]; then
			exit 1
		fi

	done

	${STRIP} --strip-unneeded ${TMPDIR}/lib/modules/*.ko 2>/dev/null
	return 0
}

glibc() {
	mkdir -p -m 755 ${TMPDIR}/lib
	install_lib ${SYSROOT_PATH}/lib/ld-2.3.6.so ${TMPDIR}/lib/ld-2.3.6.so 
	install_lib ${SYSROOT_PATH}/lib/libBrokenLocale-2.3.6.so ${TMPDIR}/lib/libBrokenLocale-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libSegFault.so ${TMPDIR}/lib/libSegFault.so
	install_lib ${SYSROOT_PATH}/lib/libanl-2.3.6.so ${TMPDIR}/lib/libanl-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libc-2.3.6.so ${TMPDIR}/lib/libc-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libcrypt-2.3.6.so ${TMPDIR}/lib/libcrypt-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libdl-2.3.6.so ${TMPDIR}/lib/libdl-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libm-2.3.6.so ${TMPDIR}/lib/libm-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libmemusage.so ${TMPDIR}/lib/libmemusage.so
	install_lib ${SYSROOT_PATH}/lib/libnsl-2.3.6.so ${TMPDIR}/lib/libnsl-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libnss_compat-2.3.6.so ${TMPDIR}/lib/libnss_compat-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libnss_dns-2.3.6.so ${TMPDIR}/lib/libnss_dns-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libnss_files-2.3.6.so ${TMPDIR}/lib/libnss_files-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libnss_hesiod-2.3.6.so ${TMPDIR}/lib/libnss_hesiod-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libnss_nis-2.3.6.so ${TMPDIR}/lib/libnss_nis-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libnss_nisplus-2.3.6.so ${TMPDIR}/lib/libnss_nisplus-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libpcprofile.so ${TMPDIR}/lib/libpcprofile.so
	install_lib ${SYSROOT_PATH}/lib/libpthread-0.10.so ${TMPDIR}/lib/libpthread-0.10.so
	install_lib ${SYSROOT_PATH}/lib/libresolv-2.3.6.so ${TMPDIR}/lib/libresolv-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/librt-2.3.6.so ${TMPDIR}/lib/librt-2.3.6.so
	install_lib ${SYSROOT_PATH}/lib/libthread_db-1.0.so ${TMPDIR}/lib/libthread_db-1.0.so
	install_lib ${SYSROOT_PATH}/lib/libutil-2.3.6.so ${TMPDIR}/lib/libutil-2.3.6.so
	ln -s ld-2.3.6.so ${TMPDIR}/lib/ld.so.1
	ln -s libBrokenLocale-2.3.6.so ${TMPDIR}/lib/libBrokenLocale.so.1
	ln -s libanl-2.3.6.so ${TMPDIR}/lib/libanl.so.1
	ln -s libc-2.3.6.so ${TMPDIR}/lib/libc.so.6
	ln -s libcrypt-2.3.6.so ${TMPDIR}/lib/libcrypt.so.1
	ln -s libdl-2.3.6.so ${TMPDIR}/lib/libdl.so.2
	ln -s libm-2.3.6.so ${TMPDIR}/lib/libm.so.6
	ln -s libnsl-2.3.6.so ${TMPDIR}/lib/libnsl.so.1
	ln -s libnss_compat-2.3.6.so ${TMPDIR}/lib/libnss_compat.so.2
	ln -s libnss_dns-2.3.6.so ${TMPDIR}/lib/libnss_dns.so.2
	ln -s libnss_files-2.3.6.so ${TMPDIR}/lib/libnss_files.so.2
	ln -s libnss_hesiod-2.3.6.so ${TMPDIR}/lib/libnss_hesiod.so.2
	ln -s libnss_nis-2.3.6.so ${TMPDIR}/lib/libnss_nis.so.2
	ln -s libnss_nisplus-2.3.6.so ${TMPDIR}/lib/libnss_nisplus.so.2
	ln -s libpthread-0.10.so ${TMPDIR}/lib/libpthread.so.0
	ln -s libresolv-2.3.6.so ${TMPDIR}/lib/libresolv.so.2
	ln -s librt-2.3.6.so ${TMPDIR}/lib/librt.so.1
	ln -s libthread_db-1.0.so ${TMPDIR}/lib/libthread_db.so.1
	ln -s libutil-2.3.6.so ${TMPDIR}/lib/libutil.so.1
}

ag7100() {
	install_lib ../ag7100/ag7100_mod.ko ${TMPDIR}/lib/modules/ag7100_mod.ko
}

avahi() {
	install_bin ../avahi/avahi-autoipd/avahi-autoipd ${TMPDIR}/usr/sbin/avahi-autoipd
	install_bin ../avahi/avahi-daemon/avahi-daemon ${TMPDIR}/usr/sbin/avahi-daemon
}

busybox() {
	install_file "../busybox/_install/*" ${TMPDIR}/ busybox
}

dhcpcd() {
	install_bin ../dhcpcd/src/dhcpcd ${TMPDIR}/sbin/dhcpcd dhcpcd
}

ntp() {
	install_bin ../ntp/sntp/sntp ${TMPDIR}/usr/sbin/sntp
}

openssl() {
	mkdir -p -m 755 ${TMPDIR}/usr/lib
	install_lib ${SYSROOT_PATH}/usr/lib/libcrypto.so.0.9.8 ${TMPDIR}/usr/lib/libcrypto.so.0.9.8
	install_lib ${SYSROOT_PATH}/usr/lib/libssl.so.0.9.8 ${TMPDIR}/usr/lib/libssl.so.0.9.8
	ln -s libcrypto.so.0.9.8 ${TMPDIR}/usr/lib/libcrypto.so
	ln -s libssl.so.0.9.8 ${TMPDIR}/usr/lib/libssl.so
}

ruby () {
	mkdir -p -m 755 ${TMPDIR}/usr/lib
	install_lib ${SYSROOT_PATH}/usr/lib/libruby.so.1.8.7 ${TMPDIR}/usr/lib/libruby.so.1.8.7
	ln -s libruby.so.1.8.7 ${TMPDIR}/usr/lib/libruby.so.1.8
	ln -s libruby.so.1.8.7 ${TMPDIR}/usr/lib/libruby.so
	mkdir -p -m 755 ${TMPDIR}/usr/lib/ruby

	CURRENT_DIR=`pwd`
	cd ${SYSROOT_PATH}/usr/lib/ruby/
	find . \
		-name '*.rb' \
		-type f \
		-exec install -D -m 0644 \{\} ${CURRENT_DIR}/${TMPDIR}/usr/lib/ruby/\{\} \;
	if [ $? != 0 ]; then
		exit 1
	fi

	find . \
		-name '*.so' \
		-type f \
		-exec install -D -m 0644 \{\} ${CURRENT_DIR}/${TMPDIR}/usr/lib/ruby/\{\} \;
	if [ $? != 0 ]; then
		exit 1
	fi

	cd ${CURRENT_DIR}
	find ${TMPDIR}/usr/lib/ruby \
		-name '*.so' \
		-type f \
		-exec ${STRIP} --strip-unneeded \{\} \;
	if [ $? != 0 ]; then
		exit 1
	fi

	install_bin ${SYSROOT_PATH}/usr/bin/ruby ${TMPDIR}/usr/bin/ruby
}

samba() {
	install_bin ../samba/source/bin/smbmanager ${TMPDIR}/usr/sbin/smbmanager
	ln -s ./smbmanager ${TMPDIR}/usr/sbin/smbd
	ln -s ./smbmanager ${TMPDIR}/usr/sbin/nmbd
	mkdir -p -m 755 ${TMPDIR}/etc/samba
	install_file ../samba/silex/smb.conf ${TMPDIR}/etc/samba/smb.conf
	install_file ../samba/silex/smb.def.conf ${TMPDIR}/etc/samba/smb.def.conf
	install_file ../samba/silex/samba.sh ${TMPDIR}/usr/sbin/samba.sh
}

thttpd() {
	install_bin ../thttpd/thttpd ${TMPDIR}/usr/sbin/thttpd
	install_bin ../thttpd/extras/htpasswd ${TMPDIR}/usr/sbin/htpasswd
}

ushare() {
	install_bin ../ushare/src/ushare ${TMPDIR}/usr/bin/ushare
}

sxcgi() {
	install_bin ../sxcgi/sxcgi ${TMPDIR}/usr/sbin/sxcgi
	mkdir -p -m 755 ${TMPDIR}/usr/share/html
	install_file "../sxhtml/*" ${TMPDIR}/usr/share/html
	install_file ../sxcgi/sxsymhtml.sh ${TMPDIR}/usr/sbin/sxsymhtml.sh
}

vsftpd() {
	mkdir -p ${TMPDIR}/usr/share/empty
	install_bin ../vsftpd/vsftpd ${TMPDIR}/usr/sbin/vsftpd
	install_file ../vsftpd/silex/vsftpd.sh ${TMPDIR}/usr/sbin/vsftpd.sh
	install_file ../vsftpd/silex/vsftpd.user_list ${TMPDIR}/etc/vsftpd.user_list
	install_file ../vsftpd/silex/vsftpd.chroot_list ${TMPDIR}/etc/vsftpd.chroot_list
	ln -s /tmpfs/etc/vsftpd.conf ${TMPDIR}/etc/vsftpd.conf
}

###########
# NAS SDK #
###########
disktype(){
	install_bin ../disktype/disktype ${TMPDIR}/usr/sbin/disktype
}

ntfs-3g() {
	install_bin ../ntfs-3g/src/ntfs-3g ${TMPDIR}/usr/sbin/ntfs-3g
	ln -s ntfs-3g ${TMPDIR}/usr/sbin/ntfsvlabel
}

mtools() {
	install_bin ../mtools/mtools ${TMPDIR}/usr/sbin/mtools
	ln -s mtools ${TMPDIR}/usr/sbin/mlabel
}

fuse() {
	install_lib ../fuse/kernel/fuse.ko ${TMPDIR}/lib/modules/fuse.ko
}

# -----------------------------------------------------------------------
# silex standard source.
# -----------------------------------------------------------------------
jcpd(){
	install_bin ../jcpd/jcpd ${TMPDIR}/usr/sbin/jcpd
}

sxutils() {
	install_bin ../sxutils/sxsysconf ${TMPDIR}/sbin/sxsysconf
	install_bin ../sxutils/sxromconf ${TMPDIR}/sbin/sxromconf
	install_bin ../sxutils/sxmkhostname ${TMPDIR}/sbin/sxmkhostname
	install_bin ../sxutils/sxusbhubctrl ${TMPDIR}/bin/sxusbhubctrl
	install_bin ../sxutils/sxwpspin ${TMPDIR}/bin/sxwpspin
	install_bin ../sxutils/sxrandinit ${TMPDIR}/sbin/sxrandinit
}

sxstorage() {
	LOCALEPATH="usr/lib/locale"
	mkdir -p -m 755 ${TMPDIR}/${LOCALEPATH}
	install_file "../sxstorage/install/*" ${TMPDIR}
	install_file "../sxstorage/locale/*" ${TMPDIR}/${LOCALEPATH}/ 
	install_file ../sxstorage/script/hotplug ${TMPDIR}/sbin/hotplug 
	install_file ../sxstorage/script/sxnasctrl ${TMPDIR}/usr/sbin/sxnasctrl 
}

sxuptp() {
	install_lib ../sxuptp/jcp.ko ${TMPDIR}/lib/modules/jcp.ko
	install_lib ../sxuptp/jcp_cmd.ko ${TMPDIR}/lib/modules/jcp_cmd.ko
	install_lib ../sxuptp/sxuptp.ko ${TMPDIR}/lib/modules/sxuptp.ko
	install_lib ../sxuptp/sxuptp_driver.ko ${TMPDIR}/lib/modules/sxuptp_driver.ko
}

sxuptp-usbmode() {
	install_lib ../sxuptp-usbmode/sxuptp_usbmode.ko \
		${TMPDIR}/lib/modules/sxuptp_usbmode.ko
}

sxcontext() {
	mkdir -p -m 755 ${TMPDIR}/usr/share/context
	install_file "../sxcontext/*.def" ${TMPDIR}/usr/share/context
	install_file "../sxcontext/*.dat" ${TMPDIR}/usr/share/context
}

atheros() {
	install_lib ../atheros/linux/net80211/wlan_wep.ko      ${TMPDIR}/lib/modules/wlan_wep.ko
	install_lib ../atheros/linux/net80211/wlan.ko          ${TMPDIR}/lib/modules/wlan.ko
	install_lib ../atheros/linux/net80211/wlan_scan_sta.ko ${TMPDIR}/lib/modules/wlan_scan_sta.ko
	install_lib ../atheros/linux/net80211/wlan_scan_ap.ko  ${TMPDIR}/lib/modules/wlan_scan_ap.ko
	install_lib ../atheros/linux/net80211/wlan_ccmp.ko     ${TMPDIR}/lib/modules/wlan_ccmp.ko
	install_lib ../atheros/linux/net80211/wlan_acl.ko      ${TMPDIR}/lib/modules/wlan_acl.ko
	install_lib ../atheros/linux/net80211/wlan_tkip.ko     ${TMPDIR}/lib/modules/wlan_tkip.ko
	install_lib ../atheros/linux/net80211/wlan_xauth.ko    ${TMPDIR}/lib/modules/wlan_xauth.ko
	install_lib ../atheros/linux/ath_hal/ath_hal.ko        ${TMPDIR}/lib/modules/ath_hal.ko
	install_lib ../atheros/linux/ath/ath_ahb.ko            ${TMPDIR}/lib/modules/ath_ahb.ko

	install_lib ../atheros/common/ratectrl/ath_rate_atheros.ko ${TMPDIR}/lib/modules/ath_rate_atheros.ko
	install_lib ../atheros/common/ath_dev/ath_dev.ko           ${TMPDIR}/lib/modules/ath_dev.ko
	install_lib ../atheros/common/dfs/ath_dfs.ko               ${TMPDIR}/lib/modules/ath_dfs.ko
	install_lib ../atheros/common/ath_pktlog/ath_pktlog.ko     ${TMPDIR}/lib/modules/ath_pktlog.ko

	for i in wlanconfig 80211stats athkey athstats athstatsclr pktlogconf pktlogdump radartool; do
		install_bin ../atheros/linux/tools/$i ${TMPDIR}/usr/sbin/$i;
	done

	install_bin ../atheros/linux/tools/hal_diag/eeprom ${TMPDIR}/usr/sbin/hal_eeprom
	install_bin ../atheros/linux/tools/hal_diag/txpow  ${TMPDIR}/usr/sbin/hal_txpow
	install_bin ../atheros/linux/tools/hal_diag/ani    ${TMPDIR}/usr/sbin/hal_ani
	install_bin ../atheros/linux/tools/hal_diag/reg    ${TMPDIR}/usr/sbin/hal_reg
	install_bin ../atheros/linux/tools/hal_diag/rfgain ${TMPDIR}/usr/sbin/hal_rfgain
}

wireless_tools() {
	install_lib ../wireless_tools/libiw.so.29 ${TMPDIR}/lib/libiw.so.29
	ln -s libiw.so.29 ${TMPDIR}/lib/libiw.so

	for i in iwconfig iwlist iwpriv ifrename iwevent iwgetid iwspy; do
		install_bin ../wireless_tools/$i ${TMPDIR}/usr/sbin/$i;
	done
}

wpa_supplicant() {
	for i in wpa_supplicant wpa_passphrase wpa_cli; do
		install_bin ../wpa_supplicant/$i ${TMPDIR}/usr/sbin/$i;
	done
}

picshare() {
	install_file "../picshare/src/pic*" ${TMPDIR}/usr/bin/
	install_file "../picshare/conf/pic*" ${TMPDIR}/etc/silex/
}

# Start Script ===============================================
# This script can execute root only.
if [ ${UID} != 0 ]; then
	echo "Error: This script can execute root user only."
	exit 1
fi

# Create initrd tmp image.
if [ -d ${TMPDIR} ]; then
	echo "Already exist ${TMPDIR}. Deleted"
	rm -rf ${TMPDIR}
fi

mkdir -m 755 ${TMPDIR}
mkdir -m 755 ${TMPDIR}/etc
mkdir -m 755 ${TMPDIR}/etc/silex
mkdir -m 755 ${TMPDIR}/lib
mkdir -m 755 ${TMPDIR}/lib/modules
mkdir -m 755 ${TMPDIR}/bin
mkdir -m 755 ${TMPDIR}/sbin
mkdir -m 755 ${TMPDIR}/usr
mkdir -m 755 ${TMPDIR}/usr/bin
mkdir -m 755 ${TMPDIR}/usr/sbin
mkdir -m 755 ${TMPDIR}/proc
mkdir -m 755 ${TMPDIR}/sys

# PREINSTALL_DIR -> TMPDIR
if [ -d ${PREINSTALL_DIR} ]; then
	cp -rp ${PREINSTALL_DIR}/* ${TMPDIR}
else
	echo "ERROR:Could not found ${PREINSTALL_DIR}" 
	exit 1
fi

for PACK in ${PACKAGE_LIST} ; do
	COMMENT=${PACK}
	LEN=${#COMMENT}
	echo -n "${COMMENT}"
	while [ ${LEN} -le 27 ]
	do
		echo -n ' '
		LEN=`expr ${LEN} + 1`
	done

	${PACK}

	echo "Success"
done

# Configure directory ownership and mode
chown -R root:root ${TMPDIR} 

# Show file size
du -h ${TMPDIR}

if [ -f ${ROOTFS_FNAME} ]; then
	rm ${ROOTFS_FNAME}
fi

mksquashfs ${TMPDIR} ${ROOTFS_FNAME} -be
if [ $? != 0 ]; then
	"Error: Could not create squashfs rootfs ${TMPDIR} -> ${ROOTFS_FNAME}"
	exit 1
fi
ls -l ${ROOTFS_FNAME}
chmod 744 ${ROOTFS_FNAME}

rm -rf ${TMPDIR}

echo "Success!"
exit 0
