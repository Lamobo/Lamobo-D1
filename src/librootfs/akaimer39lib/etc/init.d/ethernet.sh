##! /bin/sh
### BEGIN INIT INFO
# File:				ethernet.sh	
# Provides:          
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:web service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2013-1-14
### END INIT INFO


#setup ethernet parameter
#

usage()
{
	echo "Usage: $0 dhcpc(1|0) ipaddr netmask gateway firstdns backdns"
	exit 1
}

if [ $# -ne 6 ]
then
	usage
fi

if [ "$1" = "0" ];
then
	ifconfig eth0 $2 netmask $3
	route add default gw $4
	sed -i "2,\$c nameserver $5\nnameserver $6" /etc/resolv.conf
fi

exit 0
