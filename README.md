# Lamobo-D1 Development Environment

[![Build Status](https://travis-ci.org/Lamobo/Lamobo-D1.svg?branch=master)](https://travis-ci.org/Lamobo/Lamobo-D1)

## Firmware Build Steps

0. Tested OS

    - Ubuntu 12.04.3 32Bit
    - Ubuntu 12.04.3 64Bit

1. Clone

    ```
    cd ~
    git clone https://github.com/Lamobo/Lamobo-D1.git
    ```

2. Build

    ```
    ~/Lamobo-D1/build/build.sh
    ```

3. Check

    ```
    cd ~/Lamobo-D1/output
    ls -l zImage root.sqsh4 root.jffs2
    ```

4. Clean

    ```
    ~/Lamobo-D1/build/build.sh clean
    ```

## Firmware Flash Steps

0. Tested OS

    - Windows 7 Professional 32Bit
    - Windows 7 Professional 64Bit

1. Prepare

    1. [PC] Connect D1 flashing port to PC via USB cable
    2. [PC] Copy build result (zImage, root.sqsh4 and root.jffs2)
    to the directory where BurnTool.exe resides

2. Flash

    1. [PC] Start "Burn Tool" application by double-click "BurnTool.exe"
    2. [D1] Power on D1 (press and hold "function" key before power on)
    3. [PC] Hold "Boot/WPS" key and wait until "Burn Tool" displays "Ready"
    4. [PC] Start flashing by click "Start" in "Burn Tool"

3. Complete

    1. [D1] Unplug power cable
    2. [D1] Plug power cable to power on
    3. [D1] Now D1 started with the new firmware
