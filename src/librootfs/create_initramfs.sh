#!/bin/sh

#initramfs

PWD=`pwd`

echo "delete orignally rootfs.initramfs"
if [ -f "root.yaffs2" ];
	then
		rm root.yaffs2 -f
fi

if [ -f "rootfs.initramfs" ];
	then
		rm rootfs.initramfs -f
fi

sudo chmod 1777 rootfs/tmp
sudo chmod 1777 rootfs/var/tmp

echo "building rootfs.initramfs, please wait a few of Second..."
#generate initramfs
fakeroot ./gen_initramfs_list.sh $PWD/rootfs | sed '-e /\(\.svn\)/d' > rootfs.initramfs
