#!/bin/sh

cd ${DIR}

./configure \
--host=${HOST} \
--prefix=/usr \
--disable-shared \
--enable-static \
