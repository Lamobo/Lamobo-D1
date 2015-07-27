#! /bin/sh
### BEGIN INIT INFO
# Provides:          wifi soft ap 
# Required-Start:    $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description: start wifi service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-2
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=$1

set_ssid_and_password()
{
	inifile="/etc/jffs2/camera.ini"

	ssid=`awk 'BEGIN {FS="="}/\[softap\]/{a=1} a==1&&$1~/s_ssid/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
	password=`awk 'BEGIN {FS="="}/\[softap\]/{a=1} a==1&&$1~/s_password/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`

    suffix=`ifconfig wlan0  |grep HWaddr | awk  '{print $5}' |awk -F ":" '{print $4$5$6}'`
    ssid=${ssid}_${suffix}

	/etc/init.d/device_save.sh all "$ssid" $password
	echo "ssid=$ssid password=$password"
}


do_start () {
	echo "start wifi soft ap......"
	insmod /root/8188eu.ko
	set_ssid_and_password
	hostapd /etc/jffs2/hostapd.conf -B
	test -f /var/run/udhcpd.pid && rm -f /var/run/udhcpd.pid
	test -f /var/run/dhcpd.pid && rm -f /var/run/dhcpd.pid
	ifconfig wlan0 192.168.0.1 #for busybox
	route add default gw 192.168.0.1 #
	udhcpd /etc/udhcpd.conf #for busybox
	if [  -d "/sys/class/net/eth0" ]
    then
	      ifconfig eth0 down
	      ifconfig eth0 up
	 fi
	/etc/init.d/wifi_led.sh wps_led on
}

do_stop () {
	/etc/init.d/wifi_stop.sh all
}

do_restart () {
	echo "restart wifi soft ap......"
	do_stop
	do_start
}

do_status () {
	echo "status wifi soft ap......"
}

case "$MODE" in
  start)
	do_start
	;;
  restart|reload|force-reload)
	do_restart
	;;
  stop)
	do_stop
	;;
  status)
	do_status
	;;
  *)
	echo "Usage: $0 start|stop|restart|status"
	exit 3
	;;
esac

exit 0 
