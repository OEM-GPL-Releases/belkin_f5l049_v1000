#! /bin/sh

echo -n "#define KERNELENTRY 0x" > kernelentry.h
echo `grep kernel_entry $1 | cut -d ' ' -f 1 ` >> kernelentry.h
