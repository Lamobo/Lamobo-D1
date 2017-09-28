#! /bin/sh
#Control connection wifi dongle
#if installed - start wifi softap
#
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin   
status=""
while true
do

	if [ -d "/sys/class/net/wlan0" ]					#if wifi dongle installed
	then {	
			#echo '>>wifi dongle installed'
		
			#Do load wifi dongle?
			status=`ifconfig wlan0 | grep RUNNING`	
			if [ "$status" = "" ]	
			then { 
				#don't load wifi dongle
				echo '>>>wlan0 interface not load, restart softap'
				/etc/init.d/wifi_softap.sh restart
			}
			else {
				#wifi interface installed
				#echo '>>>wlan0 interface loaded'
				sleep 5	
			}
			fi
	}
	fi

	sleep 3

done

exit 0
	
