#! /bin/sh
### BEGIN INIT INFO
# File:		    udisk.sh	
# Provides:         connect to PC udisk 
#




usage()
{
	echo "Usage: $0 start|stop)"
	exit 3
}


start ()
{
	echo "start connecting PC udisk......"
	sync
	umount /mnt
	
	rmmod 8188eu
	sleep 1
	rmmod otg_hs
	sleep 1

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

	dev=`ls /dev | grep mmc`
	if [ "$dev" != "" ]
	then
		dev=`ls /dev/$dev`
		mount $dev /mnt
	fi
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

