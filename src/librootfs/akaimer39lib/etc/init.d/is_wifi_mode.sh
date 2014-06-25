#! /bin/sh
### BEGIN INIT INFO
# File:				is_wifi_mode.sh
# Provides:         check if it is wifi mode
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description 
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-9-21
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
#
#main
#

echo "$0 $@"
if [ -n "`pgrep hostapd`" ] || [ -n "`pgrep wpa_supplicant`" ]
then
	exit 1
else
	exit 0
fi


