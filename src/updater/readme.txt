Upgrade tool is mainly used to upgrade the kernel and other files placed in the bin area (such as logo), boot, ram parameters, 
support for local upgrades, http upgrade, ftp upgrade

1 local upgrade
Copy the upgrade file to the file system or sd card, and then execute the upgrade command
updater local K = <kernel path> B = <boot path> L = <logo path> D = <ram parameter path>
Example: updater local K = / mnt / sd / zImage B = / mnt / sd / nandboot.bin D = / mnt / sd / ddrpar.txt

2 http upgrade
The upgrade file on the http server, to ensure that the development board can be connected to the server, and then execute the command
updater http K = <kernel path> B = <boot path> L = <logo path> D = <ram parameter path> X = <0/1>
Example: updater http K = http: //www.a.com/zImage B = http: //www.a.com/nandboot.bin D = http: //www.a.com/ddrpar.txt X = 1

3 ftp upgrade
Put the upgrade file on the ftp server, to ensure that the development board can be connected to the server, and then execute the command
updater ftp K = <kernel path> B = <boot path> L = <logo path> D = <ram parameter path> X = <0/1> A = <ip addr> P = <port> U = <username> C = <password>
Example: updater ftp K = / update / zImage B = / update / nandboot.bin D = / update / ddrpar.txt X = 0 A = 192.168.1.100 P = 21 U = anonymous C = anonymous
Where A is the ftp server ip address, P is the ftp port number, U is the user name, C is the password

Command will pop up after a string of warning messages, enter OK and then continue to enter, or stop execution
The update process will pop up the progress of the tips and success and failure of the various stages of information ("update ..... success / failure"), and finally pop up "Update End! You Should Reboot The System", at this time, you need to restart your Development board, as long as the normal start, are generally successful upgrade.
Note:
1 K, B, L options can be selected or only one of them, but the D option must rely on B option, that is to say when upgrading ram parameters must also upgrade the boot
The format of the 2 ram parameter file is the same as the format exported by the logging tool
3 in the http upgrade and ftp upgrade, X is the check option, the value of 1 (default) select the check, for verification, the need to upgrade the file with pc-side software to deal with
