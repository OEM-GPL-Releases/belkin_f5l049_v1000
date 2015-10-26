CC       = $(CROSS_COMPILE)gcc
CFLAGS   = -Wall -O2 -D_GNU_SOURCE -D_LANBACKUP -D_PICSHARE -D_WNETLIST
INCLUDES = -I../ -I../sxstorage -I../wireless_tools
LDFLAGS  =
LIBS     = -lsx -ljcp -lsxstorage -lcrypto -lpthread
LIBPATH  = -L../libsx -L../libjcp -L../sxstorage/lib
OBJS     = jcpd.o \
           jcpd_command.o \
           jcpd_config.o \
           jcpd_nasjob.o \
           jcpd_pictjob.o \
           jcpd_subutil.o \
           jcpd_wireless.o
TARGET   = jcpd

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) $(LIBPATH) 

.c.o:
	$(CC) -c $< $(CFLAGS) $(INCLUDES)

clean:
	-rm -f $(TARGET) *~ *.BAK
