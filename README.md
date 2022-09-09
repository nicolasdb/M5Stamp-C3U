## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : With I2C communication, each slave on the bus has its own address, a hexadecimal number that allows the ESP32 to communicate with each device.  
This code will scan the I2C BUS and return your devices’ I2C address.  

> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:
- M5Stamp C3U can use any GPIO as I2C BUS. You need to define the pin you want to use with `Wire.begin(I2C_SDA, I2C_SCL);`   
I have tested on (1,0) and (6,7) _(sda, sdc)  
- But only have 1 BUS I2C. _(ESP32 can manage 2 BUS I2C)_   
Check this doc: [Multiple I2C devices (same bus, different addresses)](https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/#5)

For more I2C stuff, check [issue #2](https://github.com/nicolasdb/M5Stamp-C3U/issues/2)