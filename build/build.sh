#!/bin/bash

build_tools()
{
    echo Updating package list...
    $APT_GET update

    echo Installing tools...
    $APT_GET install lzop zip
    echo Installing libraries...
    $APT_GET install mtd-utils liblzo2-dev liblzo2-2 liblzma5

    if [ ! -d $DEV_ROOT/compiler/arm-2009q3 ]; then
        $MKDIR $DEV_ROOT/compiler
        cd $DEV_ROOT/compiler
        echo Installing compiler...
        $TAR -jxf arm-2009q3-67-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
    fi
}

clean_tools()
{
    echo Cleaning tools...
    $RM $DEV_ROOT/compiler/arm-2009q3
}

config_kernel()
{
    echo Configuring kernel...
    cd $DEV_ROOT/src/kernel
    $MKDIR $DEV_ROOT/output/kernel
    $MAKE O=$DEV_ROOT/output/kernel aimer39_ak3918_D1_defconfig
    #$MAKE O=$DEV_ROOT/output/kernel menuconfig
}

build_kernel()
{
    echo Building kernel...
    cd $DEV_ROOT/src/kernel
    $MKDIR $DEV_ROOT/output/kernel
    $MAKE O=$DEV_ROOT/output/kernel LOCALVERSION= -j$NCPU
    $CP $DEV_ROOT/output/kernel/arch/arm/boot/zImage $DEV_ROOT/output

    $MAKE O=$DEV_ROOT/output/kernel LOCALVERSION= -j$NCPU modules
    $MAKE O=$DEV_ROOT/output/kernel LOCALVERSION= -j$NCPU modules_prepare

    cd $DEV_ROOT/src/kernel/drivers/net/wireless/rtl8188EUS_rtl8189ES
    $MAKE -j$NCPU KSRC=$DEV_ROOT/output/kernel modules
    $MAKE -j$NCPU KSRC=$DEV_ROOT/output/kernel strip
    $CP 8188eu.ko $DEV_ROOT/src/librootfs/akwifilib/root
}

clean_kernel()
{
    echo Cleaning kernel...
    $RM $DEV_ROOT/output/kernel
    cd $DEV_ROOT/src/kernel
    # restore kernel/lib/libakaec.a and kernel/lib/libfha.a
    git checkout lib

    cd $DEV_ROOT/src/kernel/drivers/net/wireless/rtl8188EUS_rtl8189ES
    $MAKE -j$NCPU KSRC=$DEV_ROOT/output/kernel clean
}

config_busybox()
{
    echo Configuring busybox...
    cd $DEV_ROOT/src/busybox
    $MKDIR $DEV_ROOT/output/busybox
    $MAKE O=$DEV_ROOT/output/busybox \
        ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- \
        lamobo_d1_defconfig

    #$MAKE O=$DEV_ROOT/output/busybox \
    #    ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- menuconfig
}

build_busybox()
{
    echo Building busybox...
    cd $DEV_ROOT/src/busybox
    $MAKE O=$DEV_ROOT/output/busybox \
        ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -j$NCPU

    sudo $RM $DEV_ROOT/output/busybox/rootfs
    $RM $DEV_ROOT/output/busybox/rootfs.tar.gz
    $MAKE O=$DEV_ROOT/output/busybox \
        ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- CONFIG_PREFIX=rootfs -s install

    echo Merging with the prebuilt...
    cd $DEV_ROOT/output/busybox/rootfs
    $RM linuxrc
    sudo $TAR -zxpf $DEV_ROOT/build/rootfs_prebuilt.tgz
    sudo $MKDIR mnt
    sudo $MKDIR proc
    sudo $MKDIR sys
    sudo $MKDIR tmp
    sudo $MKDIR var
    $CHMOD 755 bin
    $CHMOD 755 etc
    $CHMOD 755 sbin
    $CHMOD 755 usr
    $CHMOD 755 usr/bin
    $CHMOD 755 usr/sbin
    cd ..

    find rootfs/ -exec sudo touch -h {} \;
    sudo $TAR -zcpf rootfs.tar.gz rootfs
    $CP rootfs.tar.gz $DEV_ROOT/src/librootfs/rootfs.tar.gz
}

clean_busybox()
{
    echo Cleaning busybox...
    sudo $RM $DEV_ROOT/output/busybox
    $RM $DEV_ROOT/src/librootfs/rootfs.tar.gz
}

build_rootfs()
{
    echo Building rootfs...
    cd $DEV_ROOT/src/ipcamera
    $MAKE
    $MAKE install
    $MAKE reinstall

    cd $DEV_ROOT/src
    $CP ipcamera/rootfs/root.jffs2 $DEV_ROOT/output
    $CP ipcamera/rootfs/root.sqsh4 $DEV_ROOT/output
}

clean_rootfs()
{
    echo Cleaning rootfs...
    cd $DEV_ROOT/src/ipcamera
    $MAKE clean
    $RM $DEV_ROOT/src/ipcamera/rootfs/rootfs.tar.gz
}

build_samples()
{
    echo Building samples...

    cd $DEV_ROOT/src/samples/gpio
    $MAKE
    $STRIP gpio-led
    $CP gpio-led $DEV_ROOT/output/local/bin

    cd $DEV_ROOT/src/samples/i2c
    $MAKE
    $STRIP i2c-test
    $CP i2c-test $DEV_ROOT/output/local/bin

    cd $DEV_ROOT/src/samples/record_audio
    $MAKE
    $STRIP record_audio
    $CP ./BUILD_record_audio_EXEC/record_audio $DEV_ROOT/output/local/bin

    cd $DEV_ROOT/src/samples/record_video
    $MAKE
    $STRIP record_video
    $CP ./BUILD_record_video_EXEC/record_video $DEV_ROOT/output/local/bin
}

clean_samples()
{
    echo Cleaning samples...

    cd $DEV_ROOT/src/samples/gpio
    $MAKE clean

    cd $DEV_ROOT/src/samples/i2c
    $MAKE clean

    cd $DEV_ROOT/src/samples/record_audio
    $MAKE clean

    cd $DEV_ROOT/src/samples/record_video
    $MAKE clean
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

    if [ -z $VERBOSE ]; then
        $MAKE -j$NCPU V=
    else
        $MAKE -j$NCPU
    fi

    $STRIP $DEV_ROOT/src/node/out/Release/node
    $CP $DEV_ROOT/src/node/out/Release/node $DEV_ROOT/output/local/bin
}

clean_node()
{
    echo Cleaning node...
    cd $DEV_ROOT/src/node
    $MAKE distclean
    $RM deps/v8/tools/jsmin.pyc
    $RM tools/gyp/pylib/gyp/__init__.pyc
    $RM tools/gyp/pylib/gyp/common.pyc
    $RM tools/gyp/pylib/gyp/generator/__init__.pyc
    $RM tools/gyp/pylib/gyp/generator/make.pyc
    $RM tools/gyp/pylib/gyp/input.pyc
    $RM tools/gyp/pylib/gyp/xcode_emulation.pyc
}

build_updater()
{
    echo Building updater...
    cd $DEV_ROOT/src/updater
    $MAKE
    $CP updater $DEV_ROOT/output/local/bin
}

clean_updater()
{
    echo Cleaning updater...
    cd $DEV_ROOT/src/updater
    $MAKE clean
}

pack_basic()
{
    echo Packing Firmware...
    cd $DEV_ROOT/output
    $RM burntool
    $MKDIR burntool
    $CP ../tool/burntool/* burntool/
    $CP zImage burntool/
    $CP root.sqsh4 burntool/
    $CP root.jffs2 burntool/
    $RM D1_Basic_$REV_ID.zip
    $ZIP D1_Basic_$REV_ID.zip burntool
}

pack_extra()
{
    echo Packing TFCard...
    cd $DEV_ROOT/output
    cat >local/readme.txt << EOF
Readme: D1 Extra Program
========================

D1 has very limited storage. To enhance rootfs, we build some extra programs.

To install these programs, just put bin folder to TFCard.
To use these programs, just login to D1 termimal, and invoke them.
EOF
    $RM D1_Extra_$REV_ID.zip
    $ZIP D1_Extra_$REV_ID.zip local
}

build_all()
{
    build_tools
    $MKDIR $DEV_ROOT/output/local/bin
    $MKDIR $DEV_ROOT/output/local/lib
    config_kernel
    build_kernel
    config_busybox
    build_busybox
    build_rootfs
    build_samples
    build_node
    build_updater
    pack_basic
    pack_extra
}

clean_all()
{
    clean_tools
    clean_kernel
    clean_busybox
    clean_rootfs
    clean_samples
    clean_node
    clean_updater
    cd $DEV_ROOT
    $RM $DEV_ROOT/output
    git status -s --ignored
}

usage()
{
    echo Usage: $0 [-v] [clean]
    echo Build Lamobo-D1 Firmware.
}

help() 
{
    echo Help:
    echo "    clean"
    echo "    build"
    echo "    build_tools"
    echo "    clean_tools"
    echo "    config_kernel"
    echo "    build_kernel"
    echo "    clean_kernel"
    echo "    config_busybox"
    echo "    build_busybox"
    echo "    clean_busybox"
    echo "    build_rootfs"
    echo "    clean_rootfs"
    echo "    build_samples"
    echo "    clean_samples"
    echo "    build_node"
    echo "    clean_node"
    echo "    build_updater"
    echo "    clean_updater"
    echo "    pack_basic"
    echo "    pack_extra"
    echo "    build_all"
    echo "    clean_all"
    echo "    usage"
    echo "    help"
}

#
# Main
#

#
# Set essential variables
#
DEV_ROOT=`dirname $0`/..
DEV_ROOT=`cd $DEV_ROOT; pwd`

REV_ID=`git rev-parse HEAD`
REV_ID=${REV_ID:0:7}

NCPU=$((`grep '^processor' /proc/cpuinfo | wc -l` * 2))

export PATH=$DEV_ROOT/compiler/arm-2009q3/bin:$PATH

#
# Parse command line
#
ACTION=
VERBOSE=
while :; do
    case $1 in
        -v)
            VERBOSE=1
            ;;
        clean)
            ACTION=clean
            break
            ;;
        build)
            ACTION=build
            break
            ;;
        build_tools)
            ACTION=build_tools
            break
            ;;
        clean_tools)
            ACTION=clean_tools
            break
            ;;
        config_kernel)
            ACTION=config_kernel
            break
            ;;
        build_kernel)
            ACTION=build_kernel
            break
            ;;
        clean_kernel)
            ACTION=clean_kernel
            break
            ;;
        config_busybox)
            ACTION=config_busybox
            break
            ;;
        build_busybox)
            ACTION=build_busybox
            break
            ;;
        clean_busybox)
            ACTION=clean_busybox
            break
            ;;
        build_rootfs)
            ACTION=build_rootfs
            break
            ;;
        clean_rootfs)
            ACTION=clean_rootfs
            break
            ;;
        build_samples)
            ACTION=build_samples
            break
            ;;
        clean_samples)
            ACTION=clean_samples
            break
            ;;
        build_node)
            ACTION=build_node
            break
            ;;
        clean_node)
            ACTION=clean_node
            break
            ;;
        build_updater)
            ACTION=build_updater
            break
            ;;
        clean_updater)
            ACTION=clean_updater
            break
            ;;
        pack_basic)
            ACTION=pack_basic
            break
            ;;
        pack_extra)
            ACTION=pack_extra
            break
            ;;
        build_all)
            ACTION=build_all
            break
            ;;
        clean_all)
            ACTION=clean_all
            break
            ;;
        usage)
            ACTION=usage
            break
            ;;
        help)
            ACTION=help
            break
            ;;
        '')
            ACTION=build
            break
            ;;
        *)
            usage
            exit
            ;;
    esac
    shift
done

#
# Config tools according to the verbosity
#
if [ -z $VERBOSE ]; then
    APT_GET="sudo apt-get -y -qq"
    RM="rm -rf"
    TAR="tar"
    MKDIR="mkdir -p"
    MAKE="make -s"
    CP="cp"
    CHMOD="sudo chmod"
    STRIP="arm-none-linux-gnueabi-strip"
    ZIP="zip -r -q"
else
    APT_GET="sudo apt-get -y -q"
    RM="rm -rfv"
    TAR="tar -v"
    MKDIR="mkdir -p -v"
    MAKE="make"
    CP="cp -v"
    CHMOD="sudo chmod -v"
    STRIP="arm-none-linux-gnueabi-strip -v"
    ZIP="zip -r -v"
fi

#
# Perform build actions
#
case $ACTION in 
    "build")
        build_all
        ;;
    "clean")
        clean_all
        ;;
    "build_tools")
        build_tools
        ;;
    "clean_tools")
        clean_tools
        ;;
    "config_kernel")
        config_kernel
        ;;
    "build_kernel")
        build_kernel
        ;;
    "clean_kernel")
        clean_kernel
        ;;
    "config_busybox")
        config_busybox
        ;;
    "build_busybox")
        build_busybox
        ;;
    "clean_busybox")
        clean_busybox
        ;;
    "build_rootfs")
        build_rootfs
        ;;
    "clean_rootfs")
        clean_rootfs
        ;;
    "build_samples")
        build_samples
        ;;
    "clean_samples")
        clean_samples
        ;;
    "build_node")
        build_node
        ;;
    "clean_node")
        clean_node
        ;;
    "build_updater")
        build_updater
        ;;
    "clean_updater")
        clean_updater
        ;;
    "pack_basic")
        pack_basic
        ;;
    "pack_extra")
        pack_extra
        ;;
    "build_all")
        build_all
        ;;
    "clean_all")
        clean_all
        ;;
    "usage")
        usage
        ;;
    "help")
        help
        ;;

esac
