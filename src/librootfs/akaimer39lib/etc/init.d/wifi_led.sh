#! /bin/sh
### BEGIN INIT INFO
# File:				wifi_led.sh	
# Provides:         control wifi status led
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:control wifi status led
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-9-6
### END INIT INFO


led=/sys/class/leds/$1
mode=$2
brightness=$3
delay_off=$3
delay_on=$4
default_br=1
default_blk=100
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

usage()
{
	echo "Usage: $0 led_device mode(on|off|blink)"
	echo "Light on led: $0 wifi_wps on"
	echo "Light off led: $0 wifi_wps off"
	echo "Flash led in 200ms: $0 wifi_wps blink 100 100"
	exit 3
}

light_on_led()
{
	echo 1 > ${led}/brightness
}

light_off_led()
{
	echo 0 > ${led}/brightness
	#echo 1 > ${led}/brightness
}

blink_led()
{
	light=`cat ${led}/brightness`
	if [ "$light" -eq "0" ]
	then
		light_on_led 1
	fi
	
	echo "timer" > ${led}/trigger
	echo $delay_off > ${led}/delay_off
	echo $delay_on > ${led}/delay_on
}

#
# main:
#

if [ "$#" -lt "2" ]
then
	usage
	exit 2
fi

case "$mode" in
	on)
		if [ -z $brightness ]
		then
			brightness=$default_br
		fi
		light_on_led $brightness
		;;
	off)
		light_off_led
		;;
	blink)

		if [ -z $delay_on ]
		then
			delay_on=$default_blk
		fi

		if [ -z $delay_off ]
		then
			delay_off=$default_blk
		fi
		blink_led
		;;
	*)
		usage
		exit 1
		;;
esac

exit 0

