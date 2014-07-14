#! /bin/sh

. ../xcommon.sh 

[ -e "python.x86" ] && {
    cp python.x86 python
} || {
    echo "python.x86 not found"
    exit 1
}

unset -v AR RANLIB LDSHARED

LDFLAGS="-L${install_dir}/lib -lz -lssl -lsqlite3" \
./configure \
    --with-system-ffi \
    --host=arm-none-linux-gnueabi \
    --build=i686-pc-linux-gnu \
    --prefix=${install_dir} \
    --enable-unicode=ucs4 \
    --with-pydebug=no \
    --with-pymalloc=no \
    --enable-shared

[ $? -eq 0 ] && {
    make HOSTPYTHON=./hostpython HOSTPGEN=./Parser/hostpgen && \
       make install HOSTPYTHON=./hostpython 
    [ $? -eq 0 ] || exit 1
}

## the make the _ctypes.o .
cur_dir=`pwd`
[ -e "${cur_dir}/._out" ] && { rm -rf "${cur_dir}/._out" ;}
mkdir -p "${cur_dir}/._out"

SPECIAL_CFLAGS="-pthread -fPIC -fno-strict-aliasing -g -O2 -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes"
INC_CFLAGS="-I${cur_dir}/Modules/_ctypes \
	-I${cur_dir}/Modules/_ctypes/libffi \
	-I${cur_dir}/Modules/_ctypes/libffi/include \
	-I${cur_dir}/Modules/_ctypes/libffi/src/arm \
	-I${cur_dir}/.\
	-I${cur_dir}/Include"

get_object_name() {
    echo $1 > /tmp/.special_tmp && 
       echo `sed 's/\.[cS]$/\.o/g' /tmp/.special_tmp` >/tmp/.special_tmp && 
         cat /tmp/.special_tmp &&
	   rm -f /tmp/.special_tmp
}

do_cp_then_make() {
    cp $1/$2 ${cur_dir}/._out/.
    ${CC} ${SPECIAL_CFLAGS} ${INC_CFLAGS} -c ${cur_dir}/._out/$2 -o ${cur_dir}/._out/`get_object_name $2`
}

sources=".:_ctypes.c \
	.:callbacks.c \
	.:callproc.c \
	.:stgdict.c \
	.:cfield.c \
	libffi/src:prep_cif.c \
	libffi/src:dlmalloc.c \
	libffi/src:closures.c \
	libffi/src/arm:ffi.c \
	libffi/src/arm:sysv.S"

objects_targets=
for source in ${sources}; do
    dir=`echo ${source} | awk -F":" '{print $1}'`
    file=`echo ${source} | awk -F":" '{print $2}'`
    do_cp_then_make "Modules/_ctypes/${dir}" ${file} || exit 1
    objects_targets="${objects_targets} ${cur_dir}/._out/`get_object_name $file`"
done
${CC} -pthread -shared ${objects_targets} -o ${cur_dir}/._out/_ctypes.so || exit 1
cp ${cur_dir}/._out/_ctypes.so ${install_dir}/lib/python2.7/lib-dynload/. 

