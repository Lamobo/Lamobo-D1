#! /bin/sh
### BEGIN INIT INFO
# File:				net_demo.sh
# Provides:         check wire insert and start ethernet,otherwise station
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description 
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2013-1-15
### END INIT INFO

mode=""
status=""

check_and_start_wlan()
{
	if [ "$mode" != "wlan" ]
	then
		mode=wlan
	echo "camera.sh stop from net_demo.sh (wlan)"
#		/etc/init.d/camera.sh stop
		/etc/init.d/wirenet.sh stop
		/etc/init.d/wifi_start.sh
	fi
}

check_and_start_eth()
{
	if [ "$mode" != "eth" ]
	then
		mode=eth
	echo "camera.sh stop from net_demo.sh (eth)"
#		/etc/init.d/camera.sh stop
		/etc/init.d/wifi_stop.sh all
		/etc/init.d/wirenet.sh start #up ethernet and wait for getting ip
	fi
}


#
#main
#

#Do load ethernet module?
if [ ! -d "/sys/class/net/eth0" ]
then
	/etc/init.d/wifi_start.sh
	exit 1
else
	#ethernet always up
	ifconfig eth0 up
	sleep 3
fi

	status=`ifconfig eth0 | grep RUNNING`
while true
do
	#check whether insert internet cable
	if [ "$status" = "" ]
	then
		#don't insert internet cable
		check_and_start_wlan		
	else
		#have inserted internet cable
		check_and_start_eth
	fi

	tmp=`ifconfig eth0 | grep RUNNING`
	if [ "$tmp" != "$status" ]
	then
		#add some delay
		for loop in 1 2
		do
			sleep 1
			tmp=`ifconfig eth0 | grep RUNNING`
		done
		status=$tmp
	fi
sleep 1
done
exit 0

