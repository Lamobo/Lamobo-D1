#!/bin/sh
### BEGIN UPDATA KERNEL
# File:				updata.sh	
# Provides:         control wifi status led
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:updata kernel zImage
# Author:			li_qing
# Email: 			li_qing@anyka.oa
# Date:				2013-06-21
### END UPDATA KERNEL

VAR1="/mnt/zImage"
VAR2="/mnt/root.sqsh4"
VAR3="/mnt/root.jffs2"

usage()
{
	echo "Usage: $0 kernel_place"
	echo "updata kernel ex: $0 /mnt/zImge "
	exit 3
}

update_kernel()
{
		echo "update zImage..., please wait a moment. And don't remove card or power-off"
		
		#led blink
		/etc/init.d/wifi_led.sh wps_led blink 500 500

		/usr/bin/updater local K=${VAR1}

		#led on after updater finished
		/etc/init.d/wifi_led.sh wps_led off
		/etc/init.d/wifi_led.sh wps_led on
}

update_squash()
{		
		echo "update root.sqsh4..."
		#led blink
		/etc/init.d/wifi_led.sh wps_led blink 500 500

		/usr/bin/updater local MTD1=${VAR2}

		#led on after updater finished
		/etc/init.d/wifi_led.sh wps_led off
		/etc/init.d/wifi_led.sh wps_led on
}

update_jffs2()
{
		echo "update root.jffs2..."
		#led blink
		/etc/init.d/wifi_led.sh wps_led blink 500 500

		/usr/bin/updater local MTD2=${VAR3}

		#led on after updater finished
		/etc/init.d/wifi_led.sh wps_led off
		/etc/init.d/wifi_led.sh wps_led on
}


#
# main:
#

NUM=$#

if [ "$NUM" -lt "1" ]
then
	usage
	exit 2
fi

i=1
while [ $i -le $NUM ]
do
	if [ "$i" = "1" ]
	then
		[ "$1" = $VAR1 ] && update_kernel
		[ "$1" = $VAR2 ] && update_squash
		[ "$1" = $VAR3 ] && update_jffs2
	fi
	if [ "$i" = "2" ]
	then
		[ "$2" = $VAR1 ] && update_kernel
		[ "$2" = $VAR2 ] && update_squash
		[ "$2" = $VAR3 ] && update_jffs2
	fi
	if [ "$i" = "3" ]
	then
		[ "$3" = $VAR1 ] && update_kernel
		[ "$3" = $VAR2 ] && update_squash
		[ "$3" = $VAR3 ] && update_jffs2
	fi
	i=$(($i+1))
done


#updater local K=/mnt/zImage B=/mnt/sd/spiboot.bin

