#! /bin/sh

. ./xcommon.sh

sources="\
    zlib-1.2.5 \
    openssl-1.0.0e \
    sqlite3-3.6.22 \
    Python-2.7.2"

#    libvdpau-0.3 \
#    curl-7.22.0 \
#    libevent-2.0.15-stable \
#    transmission-2.42 \
#    libogg-1.1.4~dfsg \
#    libexif-0.6.19 \
#    libid3tag-0.15.1b  \
#    flac-1.2.1 \
#    x264-snapshot-20120322-2245 \
#    libvorbis-1.2.3 \
#    xvidcore-1.2.2+debian \
#    ffmpeg-0.10.2 \
#    libjpeg6b-6b \
#    minidlna-1.0.23 \
#    freetype-2.3.11 \
#    lcms-1.18.dfsg \
#    Imaging-1.1.7 \
#    CherryPy-3.2.2 \
#    pyinotify-0.8.9 \
#    libxml2-2.7.6.dfsg \
#    libxml2-python-2.6.9 \
#    miniupnpc-1.6 \
#    ntp-4.2.6p5 \
#    akjpeg \
#    make-rootfs"

#    dnspython-1.7.1 \

VLOG="/tmp/popobox.error.log"

do_clean_one() {
    if [ -d $1 ] && [ -e "$1/Makefile" ]; then
        echo "clean module $1..."
	cd $1 && \
	    make clean >/dev/null 2>&1 
	   # make distclean >/dev/null 2>&1 
	cd .. 
    fi
}

do_make_one() {
    if [ -d $1 ] && [ -e "$1/xcompiler.sh" ]; then
        echo -n "compiling module $1 ..."
	if cd $1 && ./xcompiler.sh >${VLOG} 2>&1;  then 
	    echo_success "Succeed"
	else
	    echo_error "Failed"
	    exit 1
	fi
	cd ..
    else
    	echo_error "Not found $1 or it's xcompiler.sh"
    	exit 1
    fi
}

echo_process() {
    if [ $1 -eq 0 ] ; then
        echo_success "$2 succeed, Congratulations !"
        return 0
    else
        echo "Sorry, $2 aborted"
        return 1
    fi
}

do_clean_all() {
    for SOURCE_ONE in ${sources} ; do 
        do_clean_one ${SOURCE_ONE}
    done
    ./xcommon.sh clean
}

do_yaffs2() {
    AFFIX=`date -I'date' | awk -F"-" '{print "-"$1$2$3}'`
    [ -e "${rootfs_dir}/popobox${AFFIX}" ] || {
        echo_error "Warning: do compilation at first."
        return 1 
    }    

    cd "make-yaffs2" && sudo ./xcompiler.sh 
    echo_process $? "Imaging "
}

do_install_pkgs() {
    cd "pre_install_pkgs" && ./xcompiler.sh $1
    echo_process $? "install-pkgs"
}

do_compilation() {
    for source in ${sources}; do
        do_make_one $source
    done
    echo_process $? "Compiling"
}

do_all() {
    do_clean_all &&
        do_compilation  &&
            do_yaffs2
}

do_freeze_env() {
    cd "freeze" && 
        ./xcompiler.sh freeze-env "$1" "$2"
}

if [ $# -eq 0 ]; then
    do_compilation 
    exit $?
fi

case "$1"x in
"clean"x)
    do_clean_all
    ;;
"rootfs"x)
    do_compilation 
    ;;
"yaffs2"x)
    do_yaffs2
    ;;
"install-pkgs"x)
    do_install_pkgs $2
    ;;
"all"x)
    do_all   
    ;;
"show-log"x)
    cat /tmp/popobox.error.log
    ;;
"freeze-env"x)
    do_freeze_env $2 $3
    ;;
*)
    echo "Not support option, the comand format should be:"
    echo "./xcompiler.sh [ clean | rootfs | yaffs2 | install-pkgs | all | show-log | freeze-env ]"
    exit 1
esac

exit $?

