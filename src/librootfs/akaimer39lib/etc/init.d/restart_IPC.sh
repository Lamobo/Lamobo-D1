#!/bin/sh
### BEGIN restart IPC
# File:				restart_IPC.sh	
# Provides:         restart the akipcserver.
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:updata kernel zImage
# Author:
# Email:
# Date:				2013-06-21
### END restart IPC

#reboot system
#killall akipcserver
#/etc/init.d/wirenet.sh start
killall akipcserver
pid=`pgrep akipcserver`
while [ "$pid" != "" ]
do         
     sleep 0.5        
     pid=`pgrep akipcserver`
				          
done
/etc/init.d/wirenet.sh start
akipcserver &
#akipcserver &

#/etc/init.d/service.sh restart
