#!/bin/sh

#
### BEGIN INIT INFO
# File:				device_save.sh
# Brief:			Modify soft APs name and password
# Provides:          
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-1
### END INIT INFO
#

#AP_FILE=hostapd.conf
AP_FILE=/etc/jffs2/hostapd.conf
AP_NAME=$2
PASSWORD=$3
AP_PASSWORD=${PASSWORD:-$2}

save_name () {
	[ "$1" != "" ] || return 
	sh -c "sed -i 's/^ssid=.*/ssid=$1/' $AP_FILE"
}

save_password () {
	[ "$1" != "" ] || return 
	PASS=$1
	PASS=${PASS//\&/\\&}
	sh -c "sed -i 's/^wpa_passphrase=.*/wpa_passphrase=$PASS/' $AP_FILE"
}

save_all () {
	save_name "$1"
	save_password $2
}

#
#main
#

case "$1" in
	name)
		save_name $AP_NAME
		;;
	password)
		save_password $AP_PASSWORD
		;;
	all)
		save_all "$AP_NAME" $AP_PASSWORD
		;;
	*)
		echo "Usage: $0 name|password|all ..."
		exit -1
		;;
esac
	
exit 0


