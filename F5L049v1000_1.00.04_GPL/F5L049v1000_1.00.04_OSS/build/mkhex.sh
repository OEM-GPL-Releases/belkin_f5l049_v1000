#!/usr/bin/bash

ROOTFS=rootfs
HEXFILE=sxlinux.hex

if [ "$1" == "--help" ]; then
	echo 'Usage: mkhex.sh [-V]'
elif [ "$1" == "-V" ]; then
	VERBOSE='V=1'
fi

if [ -z "${ARCH}" ] || [ -z "${CROSS_COMPILE}" ]; then
	echo 'Error: Environmental variables "ARCH" "CROSS_COMPILE" are not defined'
	exit 1
fi

ROOTFS=`pwd`/$ROOTFS
if [ ! -f ${ROOTFS} ]; then
	echo "Error not found ${ROOTFS}"
	exit 1
fi

pushd ./
cd ${KERNEL_PATH}
rm -f arch/arm/boot/bootp/initrd.o
make bootpImage INITRD=${ROOTFS} ${VERBOSE}
popd

${CROSS_COMPILE}objcopy -O srec --srec-forceS3 \
	--change-addresses=-0x20000000 \
	${KERNEL_PATH}/arch/arm/boot/bootp/bootp ${HEXFILE}

echo "Success! ${HEXFILE}"

exit 0
