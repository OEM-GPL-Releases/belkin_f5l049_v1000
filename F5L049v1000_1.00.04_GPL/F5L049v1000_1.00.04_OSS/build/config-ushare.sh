#!/bin/sh

cd $1

PKG_CONFIG_PATH="$2/lib/pkgconfig"
export PKG_CONFIG_PATH

#./configure \
#--prefix=/usr \
#--enable-dlna \
#--with-libupnp-dir=/home/mips-linux/sysroot/usr \
#--with-libdlna-dir=/home/mips-linux/sysroot/usr \
#--cross-prefix=${CROSS_COMPILE} \
#--cross-compile


./configure \
--prefix=/usr \
--disable-dlna \
--with-libupnp-dir=$2 \
--cross-prefix=${CROSS_COMPILE} \
--cross-compile 

