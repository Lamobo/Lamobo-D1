#!/bin/sh


ECHO=echo
RM="rm -fr"
TAR=tar
CHOWN=chown

PWD=`pwd`
ROOTFSFILE=$PWD/rootfs.tar.gz
LOGFILE=$PWD/extractlog
ROOTFSDIR=$PWD/rootfs

CURRENT_USER=`id -un`
CURRENT_GROUP=`id -gn`

extract_error()
{
	$ECHO "failed"
	exit 1
}

$RM ${ROOTFSDIR}
$ECHO -n "Extracting rootfs..."
sudo ${TAR} zxvf ${ROOTFSFILE} >/dev/null  || extract_error
$ECHO "Done."
$ECHO -n "Changing Owner to current user..."
sudo ${CHOWN} -R ${CURRENT_USER}:${CURRENT_GROUP} $ROOTFSDIR
$ECHO "Done."
