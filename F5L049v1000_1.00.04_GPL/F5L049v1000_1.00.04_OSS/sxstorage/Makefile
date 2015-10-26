CC       = $(CROSS_COMPILE)gcc
STRIP    = $(CROSS_COMPILE)strip

SXROOT   = ../
CFLAGS   = -Wall -O2 -D_GNU_SOURCE -D_REENTRANT
LDFLAGS  = -L$(SXROOT)/libsx -Llib
INCLUDES = -I$(SXROOT)/ -I./
LIBS     = -lsxstorage -lsx -lpthread
TARGETAPP= sxstoragemanager  hotplugd  usbstorage.hotplug
TARGET   = $(TARGETAPP)

# compile objects
SXSTRGMANG_OBJ    = sxmanagedstorage.o
SXHPLUGDAEMON_OBJ = sxhotplug_daemon.o receive_thread.o mount_thread.o \
                    mount_data.o storage_data.o drive_letter.o
SXHPLUG_OBJ       = sxhotplug.o

INSTALLPATH = /usr
BINPATH     = $(INSTALLPATH)/sbin
OUTDIR      = install$(INSTALLPATH)
BINOUT      = install$(BINPATH)

HPLUGPATH   = /etc/hotplug.d/block
HPLUGOUT    = install$(HPLUGPATH)

all: $(TARGET)

sxstoragemanager: $(SXSTRGMANG_OBJ)
	-mkdir -p $(BINOUT)
	-mkdir -p $(OUTDIR)/bin
	-mkdir -p $(OUTDIR)/sbin
	$(CC) $(LDFLAGS) -o $(BINOUT)/$@ $(SXSTRGMANG_OBJ) $(LIBS)
	$(STRIP) $(BINOUT)/$@
	-ln -f -s ./$@       $(OUTDIR)/sbin/sxsambaconf
	-ln -f -s ./$@       $(OUTDIR)/sbin/sxmount
	-ln -f -s ./$@       $(OUTDIR)/sbin/sxusbport
	-ln -f -s ../sbin/$@ $(OUTDIR)/bin/sxstorageinfo
	-ln -f -s ../sbin/$@ $(OUTDIR)/bin/sxstrchr
	-ln -f -s ../sbin/$@ $(OUTDIR)/bin/sxnotify

hotplugd: $(SXHPLUGDAEMON_OBJ)
	-mkdir -p $(BINOUT)
	$(CC) $(LDFLAGS) -o $(BINOUT)/$@ $(SXHPLUGDAEMON_OBJ) $(LIBS)
	$(STRIP) $(BINOUT)/$@

usbstorage.hotplug: $(SXHPLUG_OBJ)
	-mkdir -p $(HPLUGOUT)
	$(CC) $(LDFLAGS) -o $(HPLUGOUT)/$@ $(SXHPLUG_OBJ) $(LIBS)
	$(STRIP) $(HPLUGOUT)/$@

.c.o:
	$(CC) -c $< $(CFLAGS) $(INCLUDES)

clean:
	-rm -f *~ *.BAK 
	-rm -rf install

