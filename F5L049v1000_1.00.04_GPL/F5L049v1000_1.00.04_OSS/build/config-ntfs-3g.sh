#!/bin/sh

cd ${DIR}
./configure \
--host=${HOST} \
--prefix=/usr \
--disable-debug \
--disable-pedantic \
--disable-shared \
--disable-static \
--disable-library \
--disable-ldscript \
--disable-ldconfig \
--disable-mount-helper \
--disable-really-static \
--enable-mtab \
--with-fuse=internal 
