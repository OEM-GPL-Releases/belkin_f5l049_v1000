#!/bin/sh

cd ${DIR}

./configure \
--prefix=${PWD}/_install \
--build=i686-pc-linux-gnu \
--host=${HOST} \
--disable-shared \
--enable-static \
--enable-tools \
--disable-samples \
--enable-wabserver \
--enable-client \
--enable-device
