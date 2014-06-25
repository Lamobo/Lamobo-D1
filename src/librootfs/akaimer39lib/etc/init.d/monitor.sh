#!/bin/sh

count=0
interval=12
while [ 1 ]
do
    sleep $interval
    aki_pid=`pgrep akipcserver`
    if [ "$aki_pid" = "" ]
    then
        count=`expr $count + 1`
        stime=`expr $interval / 2`
        echo $count
        if [ "$count" -gt "2" ]
        then
            akipcserver &
            count=0
            interval=12
        fi
        echo "can't locate akipcserver..stime=$stime, count=$count"
    else
        echo "akipcserver is running.." > /dev/null
    fi
done
