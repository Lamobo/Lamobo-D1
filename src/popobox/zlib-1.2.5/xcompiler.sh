#! /bin/sh

. ../xcommon.sh

unset -v CXX RANLIB LDSHARED

./configure \
	--prefix="${install_dir}" \

do_make $? && exit 0 || exit 1

