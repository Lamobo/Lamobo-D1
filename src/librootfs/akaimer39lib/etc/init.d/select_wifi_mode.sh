#! /bin/sh
### BEGIN INIT INFO
# File:				select_wifi_mode.sh
# Provides:         change wifi to station or softAP mode
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description 
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-9-17
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
mode=

check_wifi_mode()
{
	pid=
	pid=`pgrep hostapd`
	if [ "$pid" != ""  ]
	then
		mode=station
	else
		mode=softap
	fi
}

do_select_mode()
{
	echo "camera.sh stop from select_wifi_mode.sh"
#	/etc/init.d/camera.sh stop
	/etc/init.d/wifi_stop.sh all
	/etc/init.d/change_wifi_mode.sh $1
	/etc/init.d/wifi_start.sh
}

#
#main
#

echo "$0 $@"

/etc/init.d/is_wifi_mode.sh
if [ "$?" -eq "0" ]
then
	echo "You are not in wifi mode!"
	exit 1
fi

check_wifi_mode
do_select_mode $mode 

exit 0


