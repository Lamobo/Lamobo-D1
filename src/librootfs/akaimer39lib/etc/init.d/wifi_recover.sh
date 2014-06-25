#! /bin/sh
PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

#wpa_config=/etc/init.d/wpa_supplicant.conf
#ap_config=/etc/init.d/hostapd.conf
aki_config=/etc/init.d/akiserver.bak
cam_config=/etc/init.d/camera.bak

#
# main:
#
echo "recover akcamera ..."
/bin/cp -f $cam_config /etc/jffs2/camera.ini
/bin/cp -f $aki_config /etc/jffs2/akiserver.ini
/etc/init.d/restart_IPC.sh

