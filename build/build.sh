#!/bin/bash

prepare_tools()
{
    sudo apt-get install -yq lzop

    if [ `uname -m` == 'x86_64' ]; then
        sudo apt-get install -yq ia32-libs liblzo2-2:i386 liblzma5:i386
    fi

    if [ ! -d $DEV_ROOT/compiler/arm-2009q3 ]; then
        mkdir -p $DEV_ROOT/compiler
        cd $DEV_ROOT/compiler
        echo Extracting compiler...
        tar jxf arm-2009q3-67-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
    fi
}

build_kernel()
{
    echo Building kernel...
    cd $DEV_ROOT/src/kernel
    #make aimer39_ak3918_defconfig
    make LOCALVERSION=
    cd $DEV_ROOT/src
    cp -v kernel/arch/arm/boot/zImage $DEV_ROOT/output
}

build_rootfs()
{
    echo Building rootfs...
    cd $DEV_ROOT/src/ipcamera
    make
    make install
    make reinstall

    cd $DEV_ROOT/src
    cp -v ipcamera/rootfs/root.jffs2 $DEV_ROOT/output
    cp -v ipcamera/rootfs/root.sqsh4 $DEV_ROOT/output
}

#
# main
#
DEV_ROOT=`dirname $0`/..
DEV_ROOT=`cd $DEV_ROOT; pwd`

export PATH=$DEV_ROOT/compiler/arm-2009q3/bin:$PATH

mkdir -p $DEV_ROOT/output

prepare_tools
build_kernel
build_rootfs
