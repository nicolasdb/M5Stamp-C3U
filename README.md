## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : :warning: Work in progress  
The sketch will define functions to list, write, append and read a file on the SPIFFS partition.   
Then using a simple timer to read a sensor and add the value in a log.txt file. 
Pushing the button on M5stamp will print the file on serial monitor. 

> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:
- the setup in Arduino IDE was quite straight forward, but I was stuck in Pio with a problem to mount the partition.  
This was solved with the package update to 2.0.5.  
`platform_packages = 
	framework-arduinoespressif32 @ https://github.com/espressif/arduino-esp32.git#2.0.5`


For more SPIFFS stuff, check [issue #7](https://github.com/nicolasdb/M5Stamp-C3U/issues/7)