#! /bin/sh
### BEGIN INIT INFO
# File:				boa_service.sh	
# Provides:         web service 
# Required-Start:   $
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description:web service
# Author:			gao_wangsheng
# Email: 			gao_wangsheng@anyka.oa
# Date:				2012-8-8
### END INIT INFO

MODE=$1
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

usage()
{
	echo "Usage: $0 start|stop)"
	exit 3
}

stop_boa()
{
	echo "stop boa service......"
	killall boa
}

start_boa ()
{
	echo "start boa service......"
	boa
}

restart_boa ()
{
	echo "restart boa service......"
	stop_boa
	start_boa
}

#
# main:
#

case "$MODE" in
	start)
		start_boa
		;;
	stop)
		stop_boa
		;;
	restart)
		restart_boa
		;;
	*)
		usage
		;;
esac
exit 0

