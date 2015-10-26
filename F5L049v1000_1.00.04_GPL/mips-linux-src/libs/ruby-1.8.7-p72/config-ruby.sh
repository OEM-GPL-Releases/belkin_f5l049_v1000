#! /bin/bash

./configure \
--enable-shared \
--target=mips-linux \
--host=mips-linux \
--build=i686-linux \
--prefix=/usr \
ac_cv_func_setpgrp_void=yes \
optflags=-L/home/mips-linux/sysroot/usr/lib