##gpio i2c sample code.

The gpio sample makes led on D1s blinking 3 times.

To access i2c device, you can just use opensource i2c tools, or this demo.
For example how to read and write rtc(ds1307) device:
./i2c-test r 0x68 0 0x12    //rtc registed on 0x68, and we read from 0x0 to 0x12.
./i2c-test w 0x68 0x1 0x49 //set minute to 49.



