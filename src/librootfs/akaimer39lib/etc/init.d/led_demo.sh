#! /bin/sh
### BEGIN INIT INFO
# File:				job.sh	
# Provides:         background job service 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:web service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2013-1-29
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

#
# main:
#
led=off
network=none

while true
do
	#led display job
	if [ -d "/sys/class/net/eth0" ]\
		&& [ -n "`ifconfig eth0|grep RUNNING`" ]\
		&& [ -n "`ifconfig eth0|grep "inet addr:"`" ]
	then
		network=eth0
	elif [ -n "`pgrep hostapd`" ]
	then
		network=softap
	elif [ -n "`pgrep wpa_supplicant`" ] \
		&& [ -n "`ifconfig wlan0|grep RUNNING`" ]\
		&& [ -n "`pgrep udhcpc`" ]\
		&& [ -n "`ifconfig wlan0|grep "inet addr:"`" ]
	then
		network=station
	else
		network=none
	fi

	case $network in
	eth0)
		if [ "$led" != "eth0" ]
		then
			led=eth0
			/etc/init.d/wifi_led.sh rec on
		fi
		;;
	softap)
		if [ "$led" != "softap" ]
		then
			led=softap
			/etc/init.d/wifi_led.sh rec on
		fi
		;;
	station)
		if [ "$led" != "station" ]
		then
			led=station
			/etc/init.d/wifi_led.sh rec on
		fi
		;;
	none)
		if [ "$led" != "off" ]
		then
			led=off
			/etc/init.d/wifi_led.sh rec off
		fi
		;;
	*)
		led=off
		network=none
		;;
	esac
	#led display job end

	sleep 1

done

exit 0

