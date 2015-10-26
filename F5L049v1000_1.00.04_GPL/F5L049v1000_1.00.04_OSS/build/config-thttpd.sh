#!/bin/sh

cd ${DIR}

./configure \
--host=${HOST} \
--target=${TARGET} \
--prefix=/usr \
--sysconfdir=/etc \
--localstatedir=/var \
