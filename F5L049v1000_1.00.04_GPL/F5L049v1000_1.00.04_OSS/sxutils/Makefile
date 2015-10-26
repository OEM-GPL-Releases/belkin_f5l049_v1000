CC       = $(CROSS_COMPILE)gcc
STRIP    = $(CROSS_COMPILE)strip

SXROOT   = ../
CFLAGS   = -Wall -O2
LDFLAGS  =
INCLUDES = -I$(SXROOT)/
LIBS     = -L$(SXROOT)/libsx -lsx
TARGET   = sxsysconf sxromconf sxmkhostname sxusbhubctrl sxwpspin sxrandinit

# sxobjects
SXSYSCONFOBJ = sxsysconf.o
SXROMCONFOBJ = sxromconf.o
SXMKHOSTNAMEOBJ = sxmkhostname.o
SXUSBHUBCTRLOBJ = sxusbhubctrl.o
SXWPSPINOBJ = sxwpspin.o
SXRANDINITOBJ = sxrandinit.o

#INSTALLPATH = /usr/local
#BINPATH     = $(INSTALLPATH)/bin

all: $(TARGET)

release: $(TARGET)
	$(STRIP) $(TARGET)

sxsysconf: $(SXSYSCONFOBJ)
	$(CC) $(LDFLAGS) -o $@ $(SXSYSCONFOBJ) $(LIBS)

sxromconf: $(SXROMCONFOBJ)
	$(CC) $(LDFLAGS) -o $@ $(SXROMCONFOBJ) $(LIBS)

sxmkhostname: $(SXMKHOSTNAMEOBJ)
	$(CC) $(LDFLAGS) -o $@ $(SXMKHOSTNAMEOBJ) $(LIBS)

sxusbhubctrl: $(SXUSBHUBCTRLOBJ)
	$(CC) $(LDFLAGS) -o $@ $(SXUSBHUBCTRLOBJ) $(LIBS)

sxwpspin: $(SXWPSPINOBJ)
	$(CC) $(LDFLAGS) -o $@ $(SXWPSPINOBJ) $(LIBS)

sxrandinit: $(SXRANDINITOBJ)
	$(CC) $(LDFLAGS) -o $@ $(SXRANDINITOBJ) $(LIBS)

.c.o:
	$(CC) -c $< $(CFLAGS) $(INCLUDES)

clean:
	-rm -f $(TARGET) *~ *.BAK

#install:
#	mkdir -p $(BINPATH)
#	cp $(TARGET) $(BINPATH)
