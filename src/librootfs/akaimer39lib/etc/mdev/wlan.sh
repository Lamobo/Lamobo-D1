#!/bin/sh
MSG_PATH="/dev/ttySAK0"

case "$ACTION" in
	add|"")
		echo "trying to bring $MDEV up" > $MSG_PATH
		/etc/init.d/wifi_softap.sh start
		;;
	remove)
		echo "trying to bring $MDEV down" > $MSG_PATH
		/etc/init.d/wifi_softap.sh stop
		;;
esac 
