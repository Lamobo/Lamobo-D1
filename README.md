# Lamobo-D1 Development Environment

## The Latest Build

[![Build Status](https://drone.io/github.com/Lamobo/Lamobo-D1/status.png)](https://drone.io/github.com/Lamobo/Lamobo-D1/latest) [Download](https://drone.io/github.com/Lamobo/Lamobo-D1/files)

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
    ls -l D1_Basic*.zip
    ls -l D1_Extra*.zip
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
    2. [PC] Extract D1_Basic*.zip (firmware and burntool)

2. Flash

    1. [PC] Start "Burn Tool" application by double-click "BurnTool.exe"
    2. [D1] Power on D1 (press and hold "function" key before power on)
    3. [PC] Hold "Boot/WPS" key and wait until "Burn Tool" displays "Ready"
    4. [PC] Start flashing by click "Start" in "Burn Tool"
    5. [PC] [Optional] Extract D1_Extra*.zip (extra programs) to the TFCard

3. Complete

    1. [D1] Unplug power cable
    2. [D1] Plug power cable to power on
    3. [D1] Now D1 started with the new firmware
