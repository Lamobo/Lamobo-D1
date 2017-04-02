#! /bin/sh
### BEGIN INIT INFO
# File:				wirenet.sh	
# Provides:         switch wire service 
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

usage()
{
	echo "Usage: $0 start|stop|restart)"
	exit 3
}

stop()
{
	echo "stop wire switch  service......"
#	/etc/init.d/camera.sh stop
	killall -9 udhcpc
#	ifconfig eth0 down //eth0 always up
}

check_ip_and_start ()
{
	inifile="/etc/jffs2/camera.ini"
	ipaddr=

	dhcp=`awk 'BEGIN {FS="="}/\[ethernet\]/{a=1} a==1&&$1~/dhcp/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`

	if [ "$dhcp" = "0" ]
	then
		ipaddr=`awk 'BEGIN {FS="="}/\[ethernet\]/{a=1} a==1&&$1~/ipaddr/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
		
		netmask=`awk 'BEGIN {FS="="}/\[ethernet\]/{a=1} a==1&&$1~/netmask/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
	
		gateway=`awk 'BEGIN {FS="="}/\[ethernet\]/{a=1} a==1&&$1~/gateway/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`

		firstdns=`awk 'BEGIN {FS="="}/\[ethernet\]/{a=1} a==1&&$1~/firstdns/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
		
		backdns=`awk 'BEGIN {FS="="}/\[ethernet\]/{a=1} a==1&&$1~/backdns/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
		
		/etc/init.d/ethernet.sh $dhcp $ipaddr $netmask $gateway $firstdns $backdns
		status=ok
	else
		ipaddr=
		status=
	fi

	while [ "$status" = "" ]
	do
		cable=`ifconfig eth0 | grep RUNNING`
		if [ "$cable" = "" ]
		then
			echo "Network cable has been unplug!"
			return
		fi
		echo "Getting ip address..."
		killall -9 udhcpc
		udhcpc -i eth0 &
		sleep 1
		status=`ifconfig eth0 | grep "inet addr:"`
	done
	/etc/init.d/camera.sh start
}

start ()
{
	echo "start wire switch  service......"
#	ifconfig eth0 up
	check_ip_and_start
}

restart ()
{
	echo "restart wire switch  service......"
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

