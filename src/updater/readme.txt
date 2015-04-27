升级工具主要用来升级内核以及其他放在bin区的文件（如logo）、boot、ram参数，支持本地升级、http升级、ftp升级
1 本地升级
把升级文件拷贝到文件系统或者sd卡上，然后执行升级命令
updater local K=<内核路径> B=<boot路径> L=<logo路径> D=<ram参数路径>
例:updater local K=/mnt/sd/zImage B=/mnt/sd/nandboot.bin D=/mnt/sd/ddrpar.txt

2 http升级
把升级文件放到http服务器上，保证开发板可以连通到服务器上，然后执行命令
updater http K=<内核路径> B=<boot路径> L=<logo路径> D=<ram参数路径> X=<0/1>
例:updater http K=http://www.a.com/zImage B=http://www.a.com/nandboot.bin D=http://www.a.com/ddrpar.txt X=1

3 ftp升级
把升级文件放到ftp服务器上，保证开发板可以连通到服务器上，然后执行命令
updater ftp K=<内核路径> B=<boot路径> L=<logo路径> D=<ram参数路径> X=<0/1> A=<ip addr> P=<port> U=<username> C=<password>
例:updater ftp K=/update/zImage B=/update/nandboot.bin D=/update/ddrpar.txt X=0 A=192.168.1.100 P=21 U=anonymous C=anonymous
其中A是ftp服务器ip地址，P是ftp端口号，U是用户名，C是密码

命令执行后会弹出一串警告信息，输入OK然后回车继续，否则中止执行
升级过程中会弹出进度提示和各个阶段成功与失败的信息（“update .....  success/failure”）,最后弹出“Update End! You Should Reboot The System”，此时，您需要重启您的开发板，只要能够正常启动，一般都是升级成功的。
注：
1 K、B、L选项可以全选或者只选其中一个，但是D选项必须依赖B选项，也就是说升级ram参数的时候必须同时升级boot
2 ram参数文件的格式跟烧录工具导出的格式一致
3 在http升级和ftp升级的时候，X是否校验选项，值为1(默认)选择校验，如需校验，需要把升级文件用pc端的软件处理一下