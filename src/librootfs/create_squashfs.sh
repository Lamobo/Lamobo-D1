#!/bin/sh

#	第一个参数指定要打包目录
#	第二个参数指定输出文件名
#	第三个参数指定打包方式

./mksquashfs rootfs/ root.sqsh4 -noappend

