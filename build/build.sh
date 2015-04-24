#!/bin/bash

prepare_tools()
{
    sudo apt-get update -y
    sudo apt-get install -yq lzop

    if [ `uname -m` == 'x86_64' ]; then
        sudo apt-get install -yq --force-yes ia32-libs ia32-libs-multiarch liblzo2-2:i386 liblzma5:i386
    fi

    if [ ! -d $DEV_ROOT/compiler/arm-2009q3 ]; then
        mkdir -p $DEV_ROOT/compiler
        cd $DEV_ROOT/compiler
        echo Extracting compiler...
        tar jxf arm-2009q3-67-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
    fi
}

config_kernel()
{
    echo Configuring kernel...
    cd $DEV_ROOT/src/kernel
    make aimer39_ak3918_D1_defconfig
    #make menuconfig
}

build_kernel()
{
    echo Building kernel...
    cd $DEV_ROOT/src/kernel
    make LOCALVERSION=
    cd $DEV_ROOT/src
    cp -v kernel/arch/arm/boot/zImage $DEV_ROOT/output
}

clean_kernel()
{
    echo Cleaning kernel...
    cd $DEV_ROOT/src/kernel
    make -s clean
    # restore kernel/lib/libakaec.a and kernel/lib/libfha.a
    git checkout lib
}

config_busybox()
{
    echo Configuring busybox...
    cd $DEV_ROOT/src/busybox
    make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -s lamobo_d1_defconfig
    #make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -s menuconfig
}

build_busybox()
{
    echo Building busybox...
    cd $DEV_ROOT/src/busybox
    make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -s

    rm -rf rootfs
    rm rootfs.tar.gz
    make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- CONFIG_PREFIX=rootfs -s install

    echo Merging with the prebuilt...
    cd rootfs
    rm -v linuxrc
    sudo tar zxpf $DEV_ROOT/build/rootfs_prebuilt.tgz
    chmod 755 bin
    chmod 755 etc
    chmod 755 sbin
    chmod 755 usr
    chmod 755 usr/bin
    chmod 755 usr/sbin
    cd ..

    find rootfs/ -exec touch -h {} \;
    tar zcpf rootfs.tar.gz rootfs
    cp -v rootfs.tar.gz ../librootfs/rootfs.tar.gz
}

clean_busybox()
{
    echo Cleaning busybox...
    cd $DEV_ROOT/src/busybox
    make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -s distclean
    #make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -s clean
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

clean_rootfs()
{
    echo Cleaning rootfs...
    cd $DEV_ROOT/src/ipcamera
    make -s clean
}

build_samples()
{
    echo Building samples...

    cd $DEV_ROOT/src/samples/gpio
    make
    cp -v gpio-led $DEV_ROOT/output

    cd $DEV_ROOT/src/samples/i2c
    make
    cp -v i2c-test $DEV_ROOT/output

    cd $DEV_ROOT/src/samples/record_audio
    make
    cp -v ./BUILD_record_audio_EXEC/record_audio $DEV_ROOT/output

    cd $DEV_ROOT/src/samples/record_video
    make
    cp -v ./BUILD_record_video_EXEC/record_video $DEV_ROOT/output
}

clean_samples()
{
    echo Cleaning samples...

    cd $DEV_ROOT/src/samples/gpio
    make -s clean

    cd $DEV_ROOT/src/samples/i2c
    make -s clean

    cd $DEV_ROOT/src/samples/record_audio
    make -s clean

    cd $DEV_ROOT/src/samples/record_video
    make -s clean
}

#
# main
#
DEV_ROOT=`dirname $0`/..
DEV_ROOT=`cd $DEV_ROOT; pwd`

export PATH=$DEV_ROOT/compiler/arm-2009q3/bin:$PATH

mkdir -p $DEV_ROOT/output

if [ "$1" == "" ]; then
    prepare_tools
    config_kernel
    build_kernel
    config_busybox
    build_busybox
    build_rootfs
    build_samples
elif [ "$1" == "clean" ]; then
    clean_kernel
    clean_busybox
    clean_rootfs
    clean_samples
fi
