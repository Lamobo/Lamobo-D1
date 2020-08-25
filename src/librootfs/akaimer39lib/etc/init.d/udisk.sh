#! /bin/sh
### BEGIN INIT INFO
# File:		    udisk.sh	
# Provides:         connect to PC udisk 
#

MOUNT_PATH="/mnt"


usage()
{
	echo "Usage: $0 start|stop)"
	exit 3
}


start ()
{
	echo "start connecting PC udisk......"
	sync
	umount $MOUNT_PATH
	
	rmmod 8188eu
	usleep 500000
	rmmod otg_hs
	#sleep 500*1000

	dev=`ls /dev | grep mmc`
	if [ "$dev" != "" ]
	then
		dev=`ls /dev/$dev`
	fi

	if [ "$dev" != "" ]
	then
		insmod /root/udc.ko
		insmod /root/g_mass_storage.ko file=$dev stall=0 removable=1
		exit 0
	fi

	echo "no sd card"
	exit 1
}

stop ()
{
	echo "stop udisk......"
	sync

	rmmod g_mass_storage
	rmmod udc
	
	insmod /root/otg-hs.ko
	sleep 1
	insmod /root/8188eu.ko
	
	cnt=$(ls /dev | grep mmc | wc -l)	
	
	if [[ $cnt -ge 2 ]]; then
		echo "mount mmcblk0p1"
		mount -t auto /dev/mmcblk0p1 $MOUNT_PATH
		elif [[ $cnt -eq 1 ]]; then
		echo "mount mmcblk0"
		mount -t auto  /dev/mmcblk0 $MOUNT_PATH
		
	fi
	exit 0
}

#
# main:
#

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	*)
		usage
		;;
esac
exit 0

