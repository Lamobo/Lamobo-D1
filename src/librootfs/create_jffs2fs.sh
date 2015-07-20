#!/bin/sh

#	mtd-utils工具集包含创建jffs2文件系统镜像的工具mkfs.jffs2，此说明主要针对
#	此工具。
#
# 1. 编译说明
#	此软件包的依赖关系为liblzo2-2, libuuid1, zlib1g, liblzo2-dev, uuid-dev,
#	libacl1-dev, zlib1g-dev. 请编译前安装这些包。
#	然后进入mtd-utils目录，执行make指令即可产生mkfs.jffs2
#
# 2. 使用说明
#	示例：
#	./mkfs.jffs2 -d rootfs/ -s 256 -e 4096 -m none -o root.jffs2

#	-d  指定要打包的文件夹
#	-s  指定页的大小，固定使用大小为256的页，请勿修改
#	-e  指定擦除块的大小，由于SPI flash擦除大小各异，此处默认4096，如果烧录
#	失败，请联系我们
#	-m  指定压缩类型，考虑ak37的性能及内存大小，目前不使用压缩机制
#	-o  指定输出文件的名字
#
# 3. 注意事项
#	某些linux发行版肯能已经自带mkfs.jffs2工具，请不要使用它！我们对此工具作了
#	某些修改，与标准的mkfs.jffs2并不兼容。
#
# 4. 挂载方法, 获得分区的信息前需要挂载
#	mount -t jffs2 /dev/mtdblock1 /mnt  ;/dev/mtdblock0表示spi1.0分区.
#
# 目前把etc/目录作成jffs2文件系统，烧录到spi分区，在系统启动后再挂载到etc/目录下.
# 如此对rootfs/etc/目录的写操作得以保存.
./mkfs.jffs2 -d rootfs/etc -s 256 -e 4096 -X lzo -m priority -o root.jffs2
