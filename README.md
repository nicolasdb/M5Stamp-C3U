## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : :warning: Work in progress  
The sketch will give you complete understanding on how to set/read date & time on DS1307 RTC module and can serve as the basis for more practical experiments and projects.

> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:
- I followed part of [this tutorial](https://lastminuteengineers.com/ds1307-rtc-arduino-tutorial/)
- I needed to include SPI.h

But I failed to setup the SD card part of the logger module. Something wrong on the SPI side, don't know if it's the code or the module and I don't have another module to test. 

For more Data Logger stuff, check [issue #6](https://github.com/nicolasdb/M5Stamp-C3U/issues/6)