## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : Will just read the value from the sensor and return them in the serial monitor.

> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:
- I'm using this library : https://github.com/RobTillaart/AM232X
- Still need to add in setup `Wire.begin(sda,scl);` with sda = 6 et scl = 7 in my case.  

For more AM2320 stuff, check [issue #5](https://github.com/nicolasdb/M5Stamp-C3U/issues/5)