BELKIN F5L049v1000 1.00.04 Open Source Code

Contents:

- F5L049v1000_1.00.04_OSS.tar.gz
   Open source code

- mips-linux-src.tar.gz
   Toolchain source files

- mips-linux-toolchain.tar.gz
   Toolchain files

- readme.txt
   This file


Build Procedure:

Must have root authority to build programs.

1. Extract mips-linux-toolchain.tar.gz to the /home directory of
   your build environment. A directory named mips-linux will be
   created.

2. Export environmental variables as follows.

      # export PATH=$PATH:/home/mips-linux/bin
      # export ARCH=mips CROSS_COMPILE=mips-linux- TARGET=mips-linux

3. Extract F5L049v1000_1.00.04_OSS.tar.gz to your working directory.

4. Move to the F5L049v1000_1.00.04_OSS/build directory.

5. Type as follows to build a firmware file.

      # make configure
      # make

   A file named F5L049v1000_US_1.00.04, that can be uploaded to
   Home Base, will be created in the build directory.
