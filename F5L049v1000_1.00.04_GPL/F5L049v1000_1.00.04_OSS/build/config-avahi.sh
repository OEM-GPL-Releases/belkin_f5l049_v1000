#!/bin/sh

cd ${DIR}

./configure \
--host=${HOST} \
--prefix=/usr \
--sbindir=/usr/sbin \
--sysconfdir=/etc \
--localstatedir=/var \
--disable-rpath  \
--disable-nls \
--disable-shared \
--with-distro=none \
--disable-glib \
--disable-gobject \
--disable-qt3 \
--disable-qt4 \
--disable-dbus \
--disable-gdbm \
--disable-python \
--disable-pygtk \
--disable-python-dbus \
--disable-mono \
--disable-monodoc \
--disable-doxygen-doc \
--disable-doxygen-dot \
--disable-doxygen-xml \
--disable-doxygen-html \
--disable-manpages \
--disable-xmltoman \
--disable-gtk \

