CC = $(CROSS_COMPILE)gcc
LD =
CFLAGS = -Wall -O2 -D_GNU_SOURCE
LDFLAGS =
LIBPATH = -L../libsx -L../sxstorage/lib -L../wireless_tools
LDLIBS = -lsx -lsxstorage ../wireless_tools/libiw.so.29 
INCLUDEPATH = -I../ -I../sxstorage -l../wireless_tools

TARGET = sxcgi
OBJS   = sxcgi.o \
	libsxcgi.o \
	plglang.o \
	sxcgi_def.o \
	plg_basic.o \
	plg_stat.o \
	plg_post.o \
	plg_post_firmware.o \
	allow.o \

all: $(TARGET)

%.o: %.c
	$(CC) -c $(INCLUDEPATH) $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBPATH) $(LDLIBS)

clean:
	-rm -f $(TARGET)
