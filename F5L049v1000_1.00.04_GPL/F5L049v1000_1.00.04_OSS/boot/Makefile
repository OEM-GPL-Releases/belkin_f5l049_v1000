#
# create a compressed vmlinux image from the original vmlinux
#
KERNELPATH = ../linux-2.6
SYSTEM = $(KERNELPATH)/vmlinux
INCDIR = -I$(KERNELPATH)/include -I$(KERNELPATH)/include/asm/mach-generic -I$(KERNELPATH)/include/asm/mach-ar7100

HEAD = boot.o start.o head.o
OBJECTS = $(HEAD) misc.o 

CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy 

CFLAGS = $(INCDIR) -D__KERNEL__ -O2 -mips3 -mno-abicalls -Wa,-non_shared

all: $(OBJECTS) $(KERNELPATH)/System.map

boot.o: boot.S
	$(CC) $(CFLAGS) -G0 -c boot.S -o boot.o

start.o: start.S
	$(CC) $(CFLAGS) -G0 -c start.S -o start.o

head.o: head.S $(KERNELPATH)/System.map
	./script.sh $(KERNELPATH)/System.map
	$(CC) $(CFLAGS) -c head.S -o head.o

misc.o: misc.c
	$(CC) $(CFLAGS) -G 0 -c misc.c

clean:
	rm -f *.o kernelentry.h
