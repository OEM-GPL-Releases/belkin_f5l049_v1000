#!/bin/sh

cd ${DIR}

./configure \
--host=${HOST} \
--prefix=/usr \
--disable-xdf \
--disable-vold \
--disable-new-vold \
--disable-debug \
--disable-floppyd \
--disable-raw-term \
--without-x
