#!/bin/sh
HTMLDIR='/usr/share/html'
WWWDIR='/var/www'
SXCGI='/usr/sbin/sxcgi'

cd /var/cache
mkdir -p /var/cache/sxcgi

echo "Directory"
cd ${WWWDIR}
mkdir -p public/stat
mkdir -p private/conf
mkdir -p private/mainte
mkdir css
mkdir help

ln -s . en
ln -s . ja
ln -s . de
ln -s . es
ln -s . fr
ln -s . it

ln -s ${HTMLDIR}/images

echo "File"
cd ${WWWDIR}
touch ./public/index.html
touch ./public/stat/index.html
touch ./private/index.html
touch ./private/conf/index.html
touch ./private/mainte/index.html
touch ./help/index.html

echo "file"
cd "${HTMLDIR}"
find ./ \
    -name "*.htm" \
    -exec ln -s ${SXCGI} ${WWWDIR}/\{\} \; \
    -print

find ./ \
    -name "*.js" \
    -exec ln -s ${SXCGI} ${WWWDIR}/\{\} \; \
    -print

find ./ \
    -name "*.css" \
    -exec ln -s ${SXCGI} ${WWWDIR}/\{\} \; \
    -print

find ./ \
    -name "*.txt" \
    -exec ln -s ${SXCGI} ${WWWDIR}/\{\} \; \
    -print
