#!/bin/sh
#mount media
MSG_PATH="/dev/ttySAK0"
MOUNT_PATH="/mnt"


case "$ACTION" in
	add|"")
	echo "[AutoMount] mount /dev/${MDEV} $MOUNT_PATH" > $MSG_PATH
	mount -t auto ${MDEV} $MOUNT_PATH
	;;
	
	remove)
	echo "[AutoMount] umount $MOUNT_PATH" > $MSG_PATH
	umount $MOUNT_PATH
	;;
esac
exit 0
	
