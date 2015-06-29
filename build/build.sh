#!/bin/bash

prepare_tools()
{
    sudo apt-get update -y
    sudo apt-get install -yq lzop zip

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
    mkdir -p $DEV_ROOT/output/kernel
    make O=$DEV_ROOT/output/kernel aimer39_ak3918_D1_defconfig
    #make O=$DEV_ROOT/output/kernel menuconfig
}

build_kernel()
{
    echo Building kernel...
    cd $DEV_ROOT/src/kernel
    make O=$DEV_ROOT/output/kernel LOCALVERSION= -j$NCPU
    cp -v $DEV_ROOT/output/kernel/arch/arm/boot/zImage $DEV_ROOT/output
}

clean_kernel()
{
    echo Cleaning kernel...
    rm -rf $DEV_ROOT/output/kernel
    cd $DEV_ROOT/src/kernel
    # restore kernel/lib/libakaec.a and kernel/lib/libfha.a
    git checkout lib
}

config_busybox()
{
    echo Configuring busybox...
    cd $DEV_ROOT/src/busybox
    mkdir -p $DEV_ROOT/output/busybox
    make O=$DEV_ROOT/output/busybox \
        ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- \
        -s lamobo_d1_defconfig

    #make O=$DEV_ROOT/output/busybox \
    #    ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -s menuconfig
}

build_busybox()
{
    echo Building busybox...
    cd $DEV_ROOT/src/busybox
    make O=$DEV_ROOT/output/busybox \
        ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -j$NCPU

    sudo rm -rf $DEV_ROOT/output/busybox/rootfs
    rm     $DEV_ROOT/output/busybox/rootfs.tar.gz
    make O=$DEV_ROOT/output/busybox \
        ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- CONFIG_PREFIX=rootfs -s install

    echo Merging with the prebuilt...
    cd $DEV_ROOT/output/busybox/rootfs
    rm -v linuxrc
    sudo tar zxpf $DEV_ROOT/build/rootfs_prebuilt.tgz
    sudo chmod 755 bin
    sudo chmod 755 etc
    sudo chmod 755 sbin
    sudo chmod 755 usr
    sudo chmod 755 usr/bin
    sudo chmod 755 usr/sbin
    cd ..

    find rootfs/ -exec sudo touch -h {} \;
    sudo tar zcpf rootfs.tar.gz rootfs
    cp -v rootfs.tar.gz $DEV_ROOT/src/librootfs/rootfs.tar.gz
}

clean_busybox()
{
    echo Cleaning busybox...
    rm -rf $DEV_ROOT/output/busybox
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
    arm-none-linux-gnueabi-strip gpio-led
    cp -v gpio-led $DEV_ROOT/output/bin

    cd $DEV_ROOT/src/samples/i2c
    make
    arm-none-linux-gnueabi-strip i2c-test
    cp -v i2c-test $DEV_ROOT/output/bin

    cd $DEV_ROOT/src/samples/record_audio
    arm-none-linux-gnueabi-strip record_audio
    make
    cp -v ./BUILD_record_audio_EXEC/record_audio $DEV_ROOT/output/bin

    cd $DEV_ROOT/src/samples/record_video
    arm-none-linux-gnueabi-strip record_video
    make
    cp -v ./BUILD_record_video_EXEC/record_video $DEV_ROOT/output/bin
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

build_node()
{
    echo Building node...
    cd $DEV_ROOT/src/node

    AR=arm-none-linux-gnueabi-ar \
    CC=arm-none-linux-gnueabi-gcc \
    CXX=arm-none-linux-gnueabi-g++ \
    LINK=arm-none-linux-gnueabi-g++ \
        ./configure \
            --without-snapshot \
            --dest-cpu=arm --dest-os=linux

    make -s -j$NCPU

    arm-none-linux-gnueabi-strip $DEV_ROOT/src/node/out/Release/node
    cp -v $DEV_ROOT/src/node/out/Release/node $DEV_ROOT/output/bin
}

clean_node()
{
    echo Cleaning node...
    cd $DEV_ROOT/src/node
    make -s distclean
}

build_updater()
{
    echo Building updater...
    cd $DEV_ROOT/src/updater
    make -s
    cp -v updater $DEV_ROOT/output/bin
}

clean_updater()
{
    echo Building updater...
    cd $DEV_ROOT/src/updater
    make -s clean
}

pack_basic()
{
    echo Packing Firmware...
    cd $DEV_ROOT/output
    rm -rf burntool
    mkdir burntool
    cp -v ../tool/burntool/* burntool/
    cp -v zImage burntool/
    cp -v root.sqsh4 burntool/
    cp -v root.jffs2 burntool/
    git rev-parse HEAD >rev.txt
    rm -f D1_Basic_$REV_ID.zip
    zip -r D1_Basic_$REV_ID.zip burntool
}

pack_extra()
{
    echo Packing TFCard...
    cd $DEV_ROOT/output
    git rev-parse HEAD >rev.txt
    cat >bin/readme.txt << EOF
Readme: D1 Extra Program
========================

D1 has very limited storage. To enhance rootfs, we build some extra programs.

To install these programs, just put bin folder to TFCard.
To use these programs, just login to D1 termimal, and invoke them.
EOF
    rm -f D1_Extra_$REV_ID.zip
    zip -r D1_Extra_$REV_ID.zip bin
}

#
# main
#
DEV_ROOT=`dirname $0`/..
DEV_ROOT=`cd $DEV_ROOT; pwd`

REV_ID=`git rev-parse HEAD`
REV_ID=${REV_ID:0:7}

NCPU=$((`grep '^processor' /proc/cpuinfo | wc -l` * 2))

export PATH=$DEV_ROOT/compiler/arm-2009q3/bin:$PATH

mkdir -p $DEV_ROOT/output/bin

if [ "$1" == "" ]; then
    prepare_tools
    config_kernel
    build_kernel
    config_busybox
    build_busybox
    build_rootfs
    build_samples
    build_node
    build_updater
elif [ "$1" == "clean" ]; then
    clean_kernel
    clean_busybox
    clean_rootfs
    clean_samples
    clean_node
    clean_updater
else
    echo Usage: $0 [clean]
fi

pack_basic
pack_extra
