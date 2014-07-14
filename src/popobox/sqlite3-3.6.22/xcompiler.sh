#! /bin/sh

. ../xcommon.sh

./configure \
    --prefix=${install_dir} \
    --host=arm-none-linux-gnueabi \
    --disable-tcl

do_make $? && exit 0 || exit 1

