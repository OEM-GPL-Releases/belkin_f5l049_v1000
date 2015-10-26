# --------------------------------------
#  Package list
# --------------------------------------
PACKAGE_LIST = \
	kernel \
	sxstorage \
	atheros \
	ag7100 \
	avahi \
	busybox \
	dhcpcd \
	disktype \
	fuse \
	jcpd \
	mtools \
	ntfs-3g \
	ntp \
	samba \
	sxcgi \
	sxuptp \
	sxuptp-usbmode \
	sxutils \
	vsftpd \
	thttpd \
	ushare \
	wireless_tools \
	wpa_supplicant \

LIBRARY_LIST = \
	libjcp \
	libsx \
	upnp \

INSTALL_PACKAGE_LIST = \
	glibc \
	openssl \
	sxcontext \
	ruby \
	picshare \
	$(PACKAGE_LIST) \

CLEAN_LIST = \
	kernel-clean \
	atheros-clean \
	ag7100-clean \
	avahi-clean \
	busybox-clean \
	dhcpcd-clean \
	disktype-clean \
	fuse-clean \
	jcpd-clean \
	libjcp-clean \
	libsx-clean \
	mtools-clean \
	ntfs-3g-clean \
	ntp-clean \
	samba-clean \
	sxcgi-clean \
	sxstorage-clean \
	sxuptp-clean \
	sxuptp-usbmode-clean \
	sxutils-clean \
	thttpd-clean \
	upnp-clean \
	ushare-clean \
	vsftpd-clean \
	wireless_tools-clean \
	wpa_supplicant-clean \

CONFIGURE_LIST = \
	kernel-configure \
	avahi-configure \
	busybox-configure \
	dhcpcd-configure \
	fuse-configure \
	thttpd-configure \
	upnp-configure \
	ushare-configure \
	mtools-configure \
	ntfs-3g-configure \
	ntp-configure \
	samba-configure \

# --------------------------------------------------
# Defined for developer
# --------------------------------------------------
PROJECT_NAME    = Van_Halen
FW_FILENAME     = F5L049v1000_US_1.00.04
BINFORMAT       = elf32-tradbigmips
KERNEL_VERSION  = 2.6.25.17
SYSROOT_PATH    = /home/mips-linux/sysroot
ROMSTART_ADDR   = bf120000 
ROMSIZE         = ed0000
ROMSUM_OFFSET   = 400

BOOT_PATH       = ../boot
AG7100_PATH     = ../ag7100
ATHEROS_PATH    = ../atheros
KERNEL_PATH     = ../linux-2.6
KERNEL_FULL_PATH= $(shell pwd)/$(KERNEL_PATH)
AVAHI_PATH      = ../avahi
BUSYBOX_PATH    = ../busybox
DHCPCD_PATH     = ../dhcpcd
DISKTYPE_PATH   = ../disktype
FUSE_PATH       = ../fuse
MTOOLS_PATH     = ../mtools
NTFS-3G_PATH    = ../ntfs-3g
NTP_PATH        = ../ntp
SAMBA_PATH      = ../samba/source
THTTPD_PATH     = ../thttpd
UPNP_PATH       = ../libupnp
USHARE_PATH     = ../ushare
VSFTPD_PATH     = ../vsftpd
WTOOLS_PATH     = ../wireless_tools
WPA_SUPPLICANT_PATH = ../wpa_supplicant

# silex original
LIBJCP_PATH     = ../libjcp
LIBSX_PATH      = ../libsx
JCPD_PATH       = ../jcpd
SXCGI_PATH      = ../sxcgi
SXUTILS_PATH    = ../sxutils
SXUPTP_PATH     = ../sxuptp
SXUPTP_USBMODE_PATH = ../sxuptp-usbmode
SXSTORAGE_PATH  = ../sxstorage

# -------------------------------------
#  Make rules
# -------------------------------------
all        : bin
clean      : $(CLEAN_LIST)
configure  : $(CONFIGURE_LIST)

kernel:
	make -C $(KERNEL_PATH)
	make -C $(BOOT_PATH) clean
	make -C $(BOOT_PATH)

boot:
	make -C $(BOOT_PATH)

ag7100:
	echo "Nothing to do"

atheros:
	make -C $(ATHEROS_PATH)/linux/tools
	make -C $(ATHEROS_PATH)/linux/tools/hal_diag

avahi:
	make -C $(AVAHI_PATH)

busybox:
	make -C $(BUSYBOX_PATH)
	make -C $(BUSYBOX_PATH) install

dhcpcd:
	make -C $(DHCPCD_PATH)

disktype:
	make -C $(DISKTYPE_PATH)

fuse:
	make -C $(FUSE_PATH)

mtools:
	make -C $(MTOOLS_PATH)

ntfs-3g:
	make -C $(NTFS-3G_PATH)

ntp:
	make -C $(NTP_PATH)

samba:
	make -C $(SAMBA_PATH)

thttpd:
	make -C $(THTTPD_PATH)

upnp:
	make -C $(UPNP_PATH)
	make -C $(UPNP_PATH) install

ushare: upnp
	CFLAGS=-I$(shell pwd)/.. LDSX="-L$(shell pwd)/../libsx -lsx"  make -C $(USHARE_PATH)

vsftpd:
	make -C $(VSFTPD_PATH)

wireless_tools:
	make -C $(WTOOLS_PATH)

wpa_supplicant:
	make -C $(WPA_SUPPLICANT_PATH)

# -------------------------------------
#  Make silex original source
# -------------------------------------
libjcp:
	echo "Nothing to do"

libsx:
	echo "Nothing to do"

jcpd:
	make -C $(JCPD_PATH)

sxcgi:wireless_tools
	make -C $(SXCGI_PATH)

sxcontext:
	echo "Nothing to make!"

sxutils:
	make -C $(SXUTILS_PATH)

sxstorage:
	make -C $(SXSTORAGE_PATH)

sxuptp:
	echo "Nothing to do"
sxuptp-usbmode:
	echo "Nothing to do"

# -------------------------------------
#  Crean rules
# -------------------------------------
kernel-clean:
	make -C $(KERNEL_PATH) clean
	make -C $(BOOT_PATH) clean

boot-clean:
	make -C $(BOOT_PATH) clean

atheros-clean:
	make -C $(ATHEROS_PATH)/linux/tools clean
	make -C $(ATHEROS_PATH)/linux/tools/hal_diag clean

ag7100-clean:
	echo "Nothing to do"

avahi-clean:
	make -C $(AVAHI_PATH) clean

busybox-clean:
	make -C $(BUSYBOX_PATH) clean

dhcpcd-clean:
	make -C $(DHCPCD_PATH) clean

disktype-clean:
	make -C $(DISKTYPE_PATH) clean

fuse-clean:
	make -C $(FUSE_PATH) clean

mtools-clean:
	make -C $(MTOOLS_PATH) clean

ntfs-3g-clean:
	make -C $(NTFS-3G_PATH) clean

ntp-clean:
	make -C $(NTP_PATH) clean

samba-clean:
	make -C $(SAMBA_PATH) clean

thttpd-clean:
	make -C $(THTTPD_PATH) clean

upnp-clean:
	make -C $(UPNP_PATH) clean
	rm -rf $(UPNP_PATH)/_install

ushare-clean:
	make -C $(USHARE_PATH) clean

vsftpd-clean:
	make -C $(VSFTPD_PATH) clean

wireless_tools-clean:
	make -C $(WTOOLS_PATH) clean

wpa_supplicant-clean:
	make -C $(WPA_SUPPLICANT_PATH) clean

# -------------------------------------
#  Clean silex original source
# -------------------------------------
libjcp-clean:
	echo "Nothing to do"

libsx-clean:
	echo "Nothing to do"

jcpd-clean:
	make -C $(JCPD_PATH) clean

sxcgi-clean:
	make -C $(SXCGI_PATH) clean

sxutils-clean:
	make -C $(SXUTILS_PATH) clean

sxuptp-clean:
	echo "Nothing to do"
sxuptp-usbmode-clean:
	echo "Nothing to do"

sxstorage-clean:
	make -C $(SXSTORAGE_PATH) clean

# -------------------------------------
#  configure rules
# -------------------------------------
kernel-configure:
	cd $(KERNEL_PATH); make $(PROJECT_NAME)_defconfig

avahi-configure:
	DIR=$(AVAHI_PATH) HOST=$(TARGET) ./config-avahi.sh

busybox-configure:
	cp $(BUSYBOX_PATH)/silex/$(PROJECT_NAME)_defconfig $(BUSYBOX_PATH)/.config

dhcpcd-configure:
	DIR=$(DHCPCD_PATH) HOST=$(TARGET) ./config-dhcpcd.sh

fuse-configure:
	DIR=$(FUSE_PATH) HOST=$(TARGET) KERNEL_PATH=$(KERNEL_PATH) KERNEL_VERSION=$(KERNEL_VERSION) ./config-fuse.sh

ntfs-3g-configure:
	DIR=$(NTFS-3G_PATH) HOST=$(TARGET) ./config-ntfs-3g.sh

ntp-configure:
	DIR=$(NTP_PATH) HOST=$(TARGET) ./config-ntp.sh

mtools-configure:
	DIR=$(MTOOLS_PATH) HOST=$(TARGET) ./config-mtools.sh

samba-configure:
	DIR=$(SAMBA_PATH) HOST=$(TARGET) TARGE=$(TARGET) ./config-samba.sh

thttpd-configure:
	DIR=$(THTTPD_PATH) HOST=$(TARGET) ./config-thttpd.sh

upnp-configure:
	DIR=$(UPNP_PATH) HOST=$(TARGET) ./config-upnp.sh

ushare-configure: upnp
	./config-ushare.sh $(USHARE_PATH) $(UPNP_PATH)/_install

# ------------------------------------
#  silex Tools
# ------------------------------------
.PHONY: sxtools sxtools-clean

sxtools-build:
	make install_dir=$(shell pwd) -C sxtools install

sxtools-clean:
	make install_dir=$(shell pwd) -C sxtools uninstall clean

# ------------------------------------
#  Archive
# ------------------------------------
.PHONY: archive

archive: $(LIBRARY_LIST) $(PACKAGE_LIST)
	./mkarchive.sh "$(INSTALL_PACKAGE_LIST)" $(KERNEL_PATH) $(SYSROOT_PATH)

# ------------------------------------
#  HEX
# ------------------------------------
.PHONY: hex bin

HEAD=$(BOOT_PATH)/boot.o $(BOOT_PATH)/start.o $(BOOT_PATH)/head.o $(BOOT_PATH)/misc.o
OBJECTS=vmlinux.o rootfs.o
hex: $(OBJECTS) $(HEAD) sxtools-build
	$(CROSS_COMPILE)ld -static -T $(PROJECT_NAME).lds -Map $(PROJECT_NAME).map -o linuximage $(HEAD) $(OBJECTS)
	$(CROSS_COMPILE)objcopy -O srec linuximage $(PROJECT_NAME).hex
	./hexsum32 -A=$(ROMSTART_ADDR) -S=$(ROMSIZE) -C=$(ROMSUM_OFFSET) -B -W $(PROJECT_NAME).hex
	rm $(OBJECTS) linuximage

vmlinux.o: kernel
	${CROSS_COMPILE}objcopy -O binary \
	--remove-section=.reginfo \
	--remove-section=.mdebug \
	--remove-section=.comment \
	--remove-section=.note \
	--remove-section=.pdr \
	--remove-section=.options \
	--remove-section=.MIPS.options \
	$(KERNEL_PATH)/vmlinux ./vmlinux.bin
	gzip -f -9 < ./vmlinux.bin > ./vmlinux.bin.gz
	echo "SECTIONS { .data : { input_len = .; LONG(input_data_end - input_data) input_data = .; *(.data) input_data_end = .; }}" > kernel2obj.lds
	$(CROSS_COMPILE)ld -o vmlinux.o -b binary vmlinux.bin.gz -b $(BINFORMAT) -T kernel2obj.lds
	rm -f kernel2obj.lds

bin: sxtools-build hex
	./sxh2b -f $(PROJECT_NAME).hex
	mv $(PROJECT_NAME).bin $(FW_FILENAME)

rootfs.o: archive
	echo "SECTIONS { .rootfs : { *(.data) }}" > rootfs.lds
	$(CROSS_COMPILE)ld -o rootfs.o -b binary rootfs -b $(BINFORMAT) -T rootfs.lds
	rm -f rootfs.lds
