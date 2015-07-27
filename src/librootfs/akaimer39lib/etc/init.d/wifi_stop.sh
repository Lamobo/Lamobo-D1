#! /bin/sh
### BEGIN INIT INFO
# File:				wifi_stop.sh	
# Provides:         stop wifi 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:stop wifi connet service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-2
### END INIT INFO

MODE=$1
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

usage()
{
	echo "Usage: $0 mode(all|station|softap)"
	exit 3
}

stop_led()
{
	/etc/init.d/wifi_led.sh wps_led off
}

stop_station()
{
	echo "stop wifi station......"
	killall -9 wpa_supplicant
	killall -9 udhcpc
	killall -9 finish_station.sh
}

stop_softap ()
{
	echo "stop wifi soft ap......"
	killall -9 udhcpd
	killall -9 hostapd
}

stop_module ()
{
	rmmod 8188eu
}


#
# main:
#

case "$MODE" in
	all)
		stop_led
		stop_station
		stop_softap
		stop_module	
		;;
	station)
		stop_led
		stop_station
		stop_module	
		;;
	softap)
		stop_led
		stop_softap	
		stop_module	
		;;
	*)
		usage
		;;
esac

exit 0

