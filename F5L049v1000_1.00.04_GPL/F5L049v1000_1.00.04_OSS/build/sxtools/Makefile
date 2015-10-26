install_dir = $(shell pwd)

.PHONY: all clean install uninstall

all:
	make -C hexsum32_v200
	make -C sxh2b_v110

clean:
	make -C hexsum32_v200 clean
	make -C sxh2b_v110 clean

install:
	make install_dir=$(install_dir) -C hexsum32_v200 install
	make install_dir=$(install_dir) -C sxh2b_v110 install

uninstall:
	make install_dir=$(install_dir) -C hexsum32_v200 uninstall
	make install_dir=$(install_dir) -C sxh2b_v110 uninstall
