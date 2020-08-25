#! /bin/sh
### BEGIN INIT INFO
# File:				select_wire_mode.sh
# Provides:         change system to wireless or wirenet mode
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description 
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-9-12
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

usage()
{
	echo "Usage: $0 wireless|wirenet)"
	exit 3
}

#
#main
#

echo "$0" $*
case "$1" in
wireless)
	#wireless mode
	killall -9 wirenet_demo.sh
	echo camera.sh stop from select_wire_mode.sh
	#/etc/init.d/camera.sh stop
	/etc/init.d/wirenet.sh stop
	/etc/init.d/wifi_stop.sh all
	ifconfig eth0 up
	ifconfig eth0 192.168.1.2 netmask 255.255.255.0
	/etc/init.d/wifi_softap.sh start
	/etc/init.d/camera.sh start
	;;
wirenet)
	#wirenet mode
	/etc/init.d/wirenet_demo.sh &
	;;
*)
	usage
	;;
esac

exit 0


