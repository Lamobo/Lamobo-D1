#! /bin/sh

. ../xcommon.sh

./Configure \
	--prefix=${install_dir} \
	no-asm \
	no-shared \
	os/compiler:${cross_compiler_dir}/bin/arm-none-linux-gnueabi-gcc

do_make $? && exit 0 || exit 1


