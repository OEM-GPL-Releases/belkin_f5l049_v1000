#!/bin/sh

cd ${DIR}

FUSE_PATH="../fuse" \
FUSE_LIBS="-pthread -L${FUSE_PATH}/lib -lfuse -lrt -ldl" \
FUSE_CFLAGS="-D_FILE_OFFSET_BITS=64 -I${FUSE_PATH}/include" \
./configure \
--host=${HOST} \
--prefix=/usr \
--disable-shared \
--disable-debug \
--disable-test \
--disable-crypto \
--disable-gnome-vfs \
--disable-static \
--without-pic \
--enable-fuse-module

