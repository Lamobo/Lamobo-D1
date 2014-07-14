#! /bin/sh

PRE="/home/zhilongf/share/anyka/toolchains/arm-2009q3/bin/arm-none-linux-gnueabi-"
CC=${PRE}gcc \
CXX=${PRE}g++ \
./configure \
    --prefix=/home/zhilongf/poponfs/install/ext/tmp \
    --host=arm-none-linux-gnueabi \
    --enable-shared 

#then run commands: make && make install
make && make install

