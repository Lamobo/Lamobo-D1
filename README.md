Lamobo-D1s Development Environment
=================================

Firmware Build Steps
--------------------

Tested OS

    - Ubuntu 12.04.3 32Bit
    - Ubuntu 12.04.3 64Bit

1. Build

    ~/lamobo-d1s/build/build.sh

2. Check

    cd ~/lamobo-d1s/output
    ls -l zImage root.sqsh4 root.jffs2


Firmware Flash Steps
--------------------

Tested OS

    - Windows 7 Professional 64Bit

1. Prepare

    1. [PC] Connect D1s flashing port to PC via USB cable
    2. [PC] Copy build result (zImage, root.sqsh4 and root.jffs2) to
       the directory where BurnTool.exe resides

2. Flash

    1. [PC] Start "Burn Tool" application by double-click "BurnTool.exe"
    2. [D1s] Power on D1s (press and hold "function" key before power on)
    3. [PC] Hold "Boot/WPS" key and wait until "Burn Tool" displays "Ready"
    4. [PC] Start flashing by click "Start" in "Burn Tool"

3. Complete

    1. [D1s] Unplug power cable
    2. [D1s] Plug power cable to power on
    3. [D1s] Now D1 started with the new firmware
