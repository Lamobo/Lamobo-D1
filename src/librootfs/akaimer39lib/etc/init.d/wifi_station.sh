#! /bin/sh
### BEGIN INIT INFO
# File:				wifi_station.sh	
# Provides:         start and exit wifi station mode 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:start and stop wifi station
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-2
### END INIT INFO

MODE=$1
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

usage()
{
	echo "Usage: $0 mode(start|stop)"
	exit 3
}

stop_station()
{
	echo "stop wifi station......"
	/etc/init.d/wifi_stop.sh all
}

get_ini_value_and_connect()
{
	inifile="/etc/jffs2/camera.ini"

	ssid=`awk 'BEGIN {FS="="}/\[wireless\]/{a=1} a==1&&$1~/^ssid/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
	mode=`awk 'BEGIN {FS="="}/\[wireless\]/{a=1} a==1&&$1~/^mode/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
	security=`awk 'BEGIN {FS="="}/\[wireless\]/{a=1} a==1&&$1~/^security/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
	password=`awk 'BEGIN {FS="="}/\[wireless\]/{a=1} a==1&&$1~/^password/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`

	if [ "$mode" = "Ad-Hoc" ] || [ "`echo $mode|grep -i "hoc"`" != "" ]
	then
		security=adhoc
	elif [ "$security" = "NONE" ] || [ "`echo $security|grep -i "none"`" != "" ]
	then
		security=open
	elif [ "$security" = "WEP" ] || [ "`echo $security|grep -i "wep"`" != "" ]
	then
		security=wep
	else
		security=wpa
	fi
	
	echo "security=$security ssid=$ssid password=$password"
	/etc/init.d/wifi_connect.sh $security "$ssid" "$password"
}

start_station ()
{
	echo "start wifi station......"
	get_ini_value_and_connect
}


#
# main:
#

case "$MODE" in
	start)
		start_station	
		;;
	stop)
		stop_station
		;;
	restart)
		stop_station
		start_station	
		;;
	*)
		usage
		;;
esac

exit 0

