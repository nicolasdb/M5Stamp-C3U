## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : :warning: Work in progress  
The sketch will 
1. Timer > analogREAD > append to log.txt on SPIFFS
2. on PushButton > print last log on serial monitor, print last log on Oled
3. create an access point wifi and publish the log on html

next: 
- add a time value to each log
- show the logs on a graph on html

> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:


For more logger stuff, check [issue #8](https://github.com/nicolasdb/M5Stamp-C3U/issues/8)