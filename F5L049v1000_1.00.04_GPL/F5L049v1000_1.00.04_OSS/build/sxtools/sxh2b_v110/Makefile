CC = gcc
LD =
CFLAGS = -Wall -O2 -D_GNU_SOURCE
LDFLAGS = -lm
LIBPATH = 
LDLIBS = 
INCLUDEPATH = 

TARGET = sxh2b
OBJS   = sxh2b.o

install_dir = ..

.PHONY: all clean install uninstall

all: $(TARGET)

%.o: %.c
	$(CC) -c $(INCLUDEPATH) $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBPATH) -o $@ $^ $(LDLIBS)

clean:
	-rm -f $(TARGET)

install: $(TARGET)
	cp $^ $(install_dir)/

uninstall:
	rm -f $(install_dir)/$(TARGET)
