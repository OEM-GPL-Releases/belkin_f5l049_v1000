#!/bin/sh

cd ${DIR}
./configure \
--prefix=/usr \
--sysconfdir=/etc \
--localstatedir=/ \
--host=${HOST} \
--target=${TARGET} \
--enable-static \
--disable-shared \
--disable-debug \
--disable-socket-wrapper \
--disable-develope \
--disable-krb5developer \
--disable-dmalloc \
--disable-cups \
--disable-iprint \
--disable-pie \
--disable-fam \
--without-fhs \
--with-privatedir=/var/lock/samba \
--with-rootsbindir=/usr/sbin \
--with-lockdir=/var/lock/samba \
--with-piddir=/var/run \
--with-configdir=/etc/samba \
--with-logfilebase=/var/log \
--without-afs \
--without-fake-kaserver \
--without-vfs-afsacl \
--without-dce-dfs \
--without-ldap \
--without-ads \
--without-krb5 \
--without-dnsupdate \
--without-automount \
--without-smbmount \
--without-cifsmount \
--without-pam \
--without-pam_smbpass \
--without-nisplus-home \
--with-syslog \
--without-quotas \
--without-sys-quotas \
--without-utmp \
--without-libmsrpc \
--without-libaddns \
--without-libsmbclient \
--without-libsmbsharemodes \
--without-cluster-support \
--without-acl-support \
--without-aio-support \
--without-sendfile-support \
--without-winbind \
--without-included-popt \
--without-included-iniparser \
--without-python \
--enable-largefile
