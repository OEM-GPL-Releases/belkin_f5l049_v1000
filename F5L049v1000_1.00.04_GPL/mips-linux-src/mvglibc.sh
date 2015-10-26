#!/bin/sh

USRLIBPATH=/home/mips-linux/sysroot/usr/lib
LIBPATH=/home/mips-linux/sysroot/lib

mkdir ${LIBPATH}
cd ${USRLIBPATH}
mv ld-2.3.6.so ../../lib/
mv libBrokenLocale-2.3.6.so ../../lib
mv libBrokenLocale.so.1 ../../lib
mv libSegFault.so ../../lib
mv libanl-2.3.6.so ../../lib
mv libanl.so.1 ../../lib
mv libc-2.3.6.so ../../lib
mv libc.so.6 ../../lib
mv libcrypt-2.3.6.so ../../lib
mv libcrypt.so.1 ../../lib
mv libdl-2.3.6.so ../../lib
mv libdl.so.2 ../../lib
mv libm-2.3.6.so ../../lib
mv libm.so.6 ../../lib
mv libmemusage.so ../../lib
mv libnsl-2.3.6.so ../../lib
mv libnsl.so.1 ../../lib
mv libnss_compat-2.3.6.so ../../lib
mv libnss_compat.so.2 ../../lib
mv libnss_dns-2.3.6.so ../../lib
mv libnss_dns.so.2 ../../lib
mv libnss_files-2.3.6.so ../../lib
mv libnss_files.so.2 ../../lib
mv libnss_hesiod-2.3.6.so ../../lib
mv libnss_hesiod.so.2 ../../lib
mv libnss_nis-2.3.6.so ../../lib
mv libnss_nis.so.2 ../../lib
mv libnss_nisplus-2.3.6.so ../../lib
mv libnss_nisplus.so.2 ../../lib
mv libpcprofile.so ../../lib/
mv libpthread-0.10.so ../../lib
mv libpthread.so.0 ../../lib/
mv libresolv-2.3.6.so ../../lib
mv libresolv.so.2 ../../lib/
mv librt-2.3.6.so ../../lib/
mv librt.so.1 ../../lib/
mv libthread_db-1.0.so ../../lib/
mv libthread_db.so.1 ../../lib/
mv libutil-2.3.6.so ../../lib/
mv libutil.so.1 ../../lib/
cd ${LIBPATH}
ln -s ld-2.3.6.so ld.so.1
cd ${USRLIBPATH}
rm ld.so.1
ln -s ../../lib/ld-2.3.6.so ld.so.1
rm libBrokenLocale.so 
ln -s ../../lib/libBrokenLocale.so.1 libBrokenLocale.so
rm libanl.so 
ln -s ../../lib/libanl.so.1 libanl.so
rm libcrypt.so 
ln -s ../../lib/libcrypt.so.1 libcrypt.so
rm libdl.so 
ln -s ../../lib/libdl.so.2 libdl.so
rm libm.so 
ln -s ../../lib/libm.so.6 libm.so 
rm libnsl.so 
ln -s ../../lib/libnsl.so.1 libnsl.so
rm libnss_compat.so 
ln -s ../../lib/libnss_compat.so.2 libnss_compat.so
rm libnss_dns.so 
ln -s ../../lib/libnss_dns.so.2 libnss_dns.so
rm libnss_files.so 
ln -s ../../lib/libnss_files.so.2 libnss_files.so
rm libnss_hesiod.so 
ln -s ../../lib/libnss_hesiod.so.2 libnss_hesiod.so
rm libnss_nis.so 
ln -s ../../lib/libnss_nis.so.2 libnss_nis.so
rm libnss_nisplus.so 
ln -s ../../lib/libnss_nisplus.so.2 libnss_nisplus.so
rm libresolv.so 
ln -s ../../lib/libresolv.so.2 libresolv.so
rm librt.so 
ln -s ../../lib/librt.so.1 librt.so
rm libthread_db.so 
ln -s ../../lib/libthread_db.so.1 libthread_db.so
rm libutil.so 
ln -s ../../lib/libutil.so.1 libutil.so

