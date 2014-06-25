#! /bin/sh
### BEGIN INIT INFO
# File:				change_wifi_mode.sh
# Provides:         change wifi_start.sh's mode to station or softap
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
file=/etc/jffs2/camera.ini
mode=$1

usage()
{
	echo "Usage: $0 mode(softap|station)!"
}

#
#main
#

echo "$0 $@"
case $mode in
	softap)
		sed -i 's/^running.*/running                        = softap/' $file
		;;
	station)
		sed -i 's/^running.*/running                        = station/' $file
		;;
	*)
		usage
		exit 1
		;;
esac

exit 0


