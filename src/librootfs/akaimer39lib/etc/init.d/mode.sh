#!/bin/sh

inifile="/etc/jffs2/camera.ini"
mode=`awk 'BEGIN {FS="="}/\[wireless\]/{a=1} a==1&&$1~/^running/{gsub(/\"/,"",$2);gsub(/\;.*/, "", $2);gsub(/^[[:blank:]]*/,"",$2);print $2}' $inifile`
softap="softap"
statsion="station"

echo mode is  $mode

if [ $mode = "station" ]; then
    exit 1;
elif [ $mode = "softap" ]; then
    exit 2;
fi
echo hello...
exit 22
