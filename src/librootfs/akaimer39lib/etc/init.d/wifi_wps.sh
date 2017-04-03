#! /bin/sh
### BEGIN INIT INFO
# File:				wifi_wps.sh	
# Provides:         change wifi into wps mode
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-9-12
### END INIT INFO


PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
mode=

station_wps()
{
	/etc/init.d/wifi_led.sh wps_led blink 250 250
	killall -9 finish_station.sh
	wpa_cli -iwlan0 disconnect
	echo "Camera stop by wifi_wps.sh (station_wps)"
#	/etc/init.d/camera.sh stop
	echo "Please press AP's WPS key in 2 minute!"
	wpa_cli -iwlan0 wps_pbc any
	/etc/init.d/finish_station.sh &
}

softap_wps()
{
	echo "Start softap wps!"
	hostapd_cli -iwlan0 wps_pbc
}

check_wifi_mode()
{
	pid=
	pid=`pgrep hostapd`
	if [ "$pid" = ""  ]
	then
		mode=station
	else
		mode=softap
	fi
}

#
#main
#

echo "$0 $@"

/etc/init.d/is_wifi_mode.sh
if [ "$?" -eq "0" ]
then
	echo You are not in wifi mode!
	exit 1
fi

check_wifi_mode
case "$mode" in
  softap)
	  softap_wps
	  ;;
  station)
	  station_wps
	  ;;
  *)
	echo "Usage: $0 softap|station"
	exit 1
	;;
esac

exit 0 
