#!/bin/sh

cd ${DIR}

./configure \
--host=${HOST} \
--prefix=/usr \
--disable-util \
--disable-example \
--disable-shared \
--disable-ldscript \
--disable-ldconfig \
--disable-mount-helper \
--disable-mtab \
--disable-rpath \
--without-libiconv-prefix \
--without-pic \
--without-pkgconfigdir \
--enable-kernel-module \
--enable-lib \
--enable-static \
--with-kernel=${KERNEL_PATH} \
--with-kernel-version=${KERNEL_VERSION}
