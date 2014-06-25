#! /bin/sh
### BEGIN INIT INFO
# File:				wifi_start.sh	
# Provides:         start wifi and  camera
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:start wifi run at station or softAP
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-8
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=

usage()
{
    echo "Usage: $0 "
}

#
# main:
#

MODE=`awk 'BEGIN {FS="="}/\[wireless\]/{a=1} a==1&&$1~/running/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' /etc/jffs2/camera.ini`

case "$MODE" in
    station)
        if [ $# == 1 ] && [ $1 = "keypress" ]
        then #by key pressed
            echo key Pressed station.
            /etc/init.d/wifi_station.sh start 
        else # by initrd..
            /etc/init.d/camera.sh start 
        fi
        ;;
    softap)
        if [ $# == 1 ] && [ $1 = "keypress" ]
        then
            echo key pressed softap..
            /etc/init.d/wifi_softap.sh start
        fi
        /etc/init.d/camera.sh restart 
        ;;
    *)
        usage
        exit 3
        ;;
esac

exit 0

