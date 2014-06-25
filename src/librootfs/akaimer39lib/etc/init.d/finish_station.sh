#! /bin/sh
### BEGIN INIT INFO
# File:				finish_station.sh	
# Provides:         check if finish wifi connection 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-10-12
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

check_ip_and_start ()
{
	status=
	while [ "$status" = "" ] 
	do
		echo "Getting ip address..."
		killall -9 udhcpc
		udhcpc -i wlan0
		status=`ifconfig wlan0 | grep "inet addr:"`
	done

	if [  -d "/sys/class/net/eth0" ]
	then
	 ifconfig eth0 down
	 ifconfig eth0 up
	fi
	/etc/init.d/camera.sh start
	/etc/init.d/wifi_led.sh wps_led blink 480 480
}
check_finish_connect()
{
	tcount=120
	status=""
	while [ "$status" != "wpa_state=COMPLETED" ]
	do
		#connect 2 min in one time
		if [ $((tcount--)) -le 0 ]
		then
			echo "Connect fail in station mode,Reconnect!"
			tcount=120
			/etc/init.d/wifi_station.sh restart
		fi

		station=`pgrep wpa_supplicant`
		if [ "$station" = "" ]
		then
			echo "Exit in station mode!"
			return 1
		fi
		
		sleep 1
		status=`wpa_cli -iwlan0 status |grep wpa_state`
	done

	check_ip_and_start 
	return 0
}

#
# main:
#

check_finish_connect
supplicant=`pgrep wpa_supplicant`
while [ "$supplicant" != "" ] 
do
	stat=`wpa_cli -iwlan0 status |grep wpa_state`
	if [ "$stat" != "wpa_state=COMPLETED" ]
	then
		/etc/init.d/wifi_led.sh wps_led blink 250 250
		/etc/init.d/camera.sh stop
		check_finish_connect
	fi

	sleep 3
	supplicant=`pgrep wpa_supplicant`
done

	/etc/init.d/wifi_led.sh wps_led off
exit 0

