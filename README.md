Lamobo-D1 Development Environment
=================================

Firmware Build Steps
--------------------

Tested OS

    - Ubuntu 12.04.3 32Bit
    - Ubuntu 12.04.3 64Bit

1. Build

    ~/lamobo-d1/build/build.sh

2. Check

    cd ~/lamobo-d1/output
    ls -l zImage root.sqsh4 root.jffs2


Firmware Flash Steps
--------------------

Tested OS

    - Windows 7 Professional 64Bit

1. Prepare

    1. [D1] Make sure 2 DIP switches are both set "Off"
    2. [PC] Connect D1 flashing port to PC via USB cable
    3. [PC] Copy build result (zImage, root.sqsh4 and root.jffs2) to
       the directory where BurnTool.exe resides

2. Flash

    1. [PC] Start "Burn Tool" application by double-click "BurnTool.exe"
    2. [D1] Power on D1 (press and hold "Boot/WPS" key before power on)
    3. [PC] Hold "Boot/WPS" key and wait until "Burn Tool" displays "Ready"
    4. [PC] Start flashing by click "Start" in "Burn Tool"
    5. [D1] Make sure 2 DIP switches are both set "On"

3. Complete

    1. [D1] Unplug power cable
    2. [D1] Plug power cable to power on
    3. [D1] Now D1 started with the new firmware
