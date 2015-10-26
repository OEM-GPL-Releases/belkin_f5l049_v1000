CC     = gcc
TARGET = hexsum32

OBJS = hexsum32.o

install_dir = ..

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $^ $(install_dir)/

uninstall:
	rm -f $(install_dir)/$(TARGET)
