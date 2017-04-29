#! /bin/sh
### BEGIN INIT INFO
# File:				camera.sh	
# Provides:         camera service 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:web service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-8
### END INIT INFO

MODE=$1
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
mode=hostapd
network=
usage()
{
	echo "Usage: $0 start|stop)"
	exit 3
}

stop()
{
	echo "stop camera service......"
	#killall akipcserver
	killall -2 record_video
	
	pid=`pgrep record_video`
	while [ "$pid" != "" ]
	do         
	    sleep 0.5        
		pid=`pgrep record_video`
   done
}

start ()
{
	echo "start camera service......"
	/usr/bin/record_video -p /mnt/ -w 720 -h 576 &
	exit 0
	#network=`pgrep hostapd`
	#if [ "$network" = "" ]
	#then
#		echo "LinuxMediaRecorder -a -r /sd_test -p udp -i 172.19.10.125 -d 5220 -C 300"
#		LinuxMediaRecorder -a -r /sd_test -p udp -i 172.19.10.125 -d 5220 -C 300 &
	#	akipcserver &
	#	exit 0
	#else
#		echo "LinuxMediaRecorder -a -r /sd_test -p udp -i 172.19.10.125 -d 5220 -C 300 -S"	
#		LinuxMediaRecorder -a -r /sd_test -p udp -i 172.19.10.125 -d 5220 -C 300 -S &
	#	akipcserver &
#		exit 1
	#fi
}

restart ()
{
	echo "restart camera service......"
	stop
	start
}

#
# main:
#

case "$MODE" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		restart
		;;
	*)
		usage
		;;
esac
exit 0

