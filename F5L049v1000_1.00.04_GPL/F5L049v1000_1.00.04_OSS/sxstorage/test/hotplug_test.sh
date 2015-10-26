#!/bin/sh

alph="abcdefghijklmnopqrstuvwxyz"
seqnum=1

# please set minor number and major number
# /proc/partitions

minor=1
major=8
MAX=10000
HPLUG="/etc/hotplug.d/block/usbstorage.hotplug"
TMP="/tmp"

if [ ${UID} != 0 ]; then
	echo "Error: This script can execute root user only."
	exit 1
fi

dnum=`expr $minor / 16 + 1`
part=`expr $minor % 16`
dletter=`echo $alph | cut -c $dnum`
if [ $part -eq 0 ]; then
	echo "can not use partition = 0"
	exit 2
fi


file=${TMP}/sd${dletter}${part}.info
if [ ! -f $file ]; then
	echo "$file is not found."
	exit 3
fi

echo $file

TMP=`sxsysconf -f ${file} SCSIPHYSPATH`
if [ -z $TMP ]; then
	echo "SCSIPHYSPATH is empty"
	exit 4
fi
physdevpath=`echo $TMP | cut -c 6-`
if [ -z $physdevpath ]; then
	echo "physdevpath is empty"
	exit 5
fi


TMP=`sxsysconf -f ${file} BLOCKPHYSPATH`
if [ -z $TMP ]; then
	echo "BLOCKPHYSPATH is empty"
	exit 6
fi
devpath=`echo $TMP | cut -c 6-`
if [ -z $devpath ]; then
	echo "devpath is empty"
	exit 7
fi


while [ $seqnum -le $MAX ];
do

	TMP=`expr $seqnum % 2`
	if [ $TMP -eq 0 ]; then
		action="add"
	else
		action="remove"
	fi

	export SEQNUM=$seqnum
	export MAJOR=$major
	export MINOR=$minor
	export ACTION=$action
	export DEVPATH=$devpath
	export PHYSDEVPATH=$physdevpath

	echo $seqnum
	$HPLUG

	sleep 5

	seqnum=`expr $seqnum + 1`
done

