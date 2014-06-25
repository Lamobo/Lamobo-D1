#!/bin/sh

# ~~~~~~~~~~~~ build initramfs ~~~~~~~~~~~~~~

PWD=`pwd`
MKYAFFSIMAGE=${PWD}/mkyaffsimage
MKYAFFS2IMAGE=${PWD}/mkyaffs2image

NUM=$#
#echo "num = ${NUM}"

case ${NUM} in
	0) 
	echo "build initramfs, please remove gloss if you need."
#	[ -f "rootfs.initramfs" ] && rm rootfs.initramfs -f

#	echo "building rootfs.initramfs, please wait a few of Second..."
	#generate initramfs
#	fakeroot ./gen_initramfs_list.sh $PWD/rootfs | sed '-e /\(\.svn\)/d' > rootfs.initramfs
#	echo "build initramfs finish."
	;;
	1)
	if [ $1 -ge 2048 ]; then
		[ -f "root.yaffs2" ] && rm root.yaffs2 -f
		case $1 in
			2048|4096|8192)
			echo "building root.yaffs2, please wait a few of Second..."
			${MKYAFFS2IMAGE} rootfs root.yaffs2 $1
			echo "building root.yaffs2 finish!"
			;;
			*)
			echo "Error, page_size must be [2048 or 4096 or 8192]"
		esac
	elif [ $1 -le 1024 ]; then
		[ -f "root.yaffs" ] && rm root.yaffs -f
		case $1 in
			512|1024)
			echo "building root.yaffs, please wait a few of Second..."
			${MKYAFFSIMAGE} rootfs root.yaffs $1
			echo "building root.yaffs finish!"
			;;
			*)
			echo "Error, page_size must be [512 or 1024]"
		esac
	else
		echo "Error, check the page_size, please input again!"
	fi
	;;
	3)
	if [ -d $1 ]; then
		if [ $3 -ge 2048 ]; then
			[ -f "root.yaffs2" ] && rm root.yaffs2 -f
			case $3 in
				2048|4096|8192)
				echo "building root.yaffs2, please wait a few of Second..."
				${MKYAFFS2IMAGE} $1 $2 $3
				echo "building root.yaffs2 finish!"
				;;
				*)
				echo "Error, page_size must be [2048 or 4096 or 8192]"
			esac
		elif [ $3 -le 1024 ]; then 	
			[ -f "root.yaffs" ] && rm root.yaffs -f
			case $3 in
				512|1024)
				echo "building root.yaffs, please wait a few of Second..."
				${MKYAFFSIMAGE} $1 $2 $3
				echo "building root.yaffs finish!"
				;;
				*)
				echo "Error, page_size must be [512 or 1024]"
			esac
		else
			echo "Error, check the page_size, please input again!"
		fi
	else
		echo "Error, file system directory is not exist."
	fi
	;;
	*)
	echo "usage one of the follow:"
	echo "./create_fs_image.sh"
	echo "./create_fs_image.sh NAND_PAGE_SIZE(512/1024/2048/4096/8192)"
	echo "./create_fs_image.sh ROOTFS_DIR ROOFS_IMAGE_NAME NAND_PAGE_SIZE(512/1024/2048/4096/8192)"
	exit 0

esac

