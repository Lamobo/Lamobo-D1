#! /bin/sh

get_pymod_lib() {
    lib_dir=`ls ./build | grep lib`
    lib_dir_count=`ls ./build | grep lib | wc -l`
    echo "./build/$lib_dir"
}

get_pymod_scripts() {
    scripts_dir=`ls ./build | grep scripts`
    scripts_dir_count=`ls ./build | grep scripts | wc -l`
    echo "./build/$scripts_dir"
}

do_make() {
    [ $1 -eq 0 ] && {
        make && make install
	return $?
    }
    return 1
}

echo_success() {
    echo "\033[0;32;1m$1\033[0m"
}

echo_error() {
    echo "\033[0;31;1m$1\033[0m"
}

DEV_ROOT=`dirname $0`/../..
DEV_ROOT=`cd $DEV_ROOT; pwd`

[ -n "${xcompiler_dir}" ] || {
    export xcompiler_dir="$DEV_ROOT/compiler/arm-2009q3"
}
[ -n "${prefix_dir}" ] || {
    export prefix_dir="$DEV_ROOT/src/popobox"
}

export cross_compiler_dir=${xcompiler_dir}
export install_dir="${prefix_dir}/install"
export rootfs_dir="${prefix_dir}/rootfs"
export python_pkgs_install_dir="${install_dir}/python-pkgs"

[ -e "${python_pkgs_install_dir}" ] || {
    mkdir -p "${python_pkgs_install_dir}"
}

export CC="${cross_compiler_dir}/bin/arm-none-linux-gnueabi-gcc"
export CXX="${cross_compiler_dir}/bin/arm-none-linux-gnueabi-g++"
export AR="${cross_compiler_dir}/bin/arm-none-linux-gnueabi-ar"
export RANLIB="${cross_compiler_dir}/bin/arm-none-linux-gnueabi-ranlib"
export LDSHARED="${cross_compiler_dir}/bin/arm-none-linux-gnueabi-gcc"
export CFLAGS="-I${cross_compiler_dir}/arm-none-linux-gnueabi/libc/usr/include -I${install_dir}/include"

if [ -n "$1" ] && [ "$1"x = "clean"x ]; then
    echo "clean install and rootfs ..."
    rm -rf ${install_dir}
    sudo rm -rf ${rootfs_dir}
fi
