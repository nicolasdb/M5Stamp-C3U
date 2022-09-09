## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : This code will connect to the DHT11 (from keyestudio) and return the temp+humidity value on the OLED 0.96 display trought the I2C BUS. And I added a custom bitmap.   


![image](https://user-images.githubusercontent.com/12049360/187668503-d94c14e6-6ff3-445c-b376-b43ce862f8cd.png)
_the picture show a MCP23017 to expand GPIO but it's not used in this code_

> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:
- It's a pain in the ass to work with I2C libraries, most recent lib didn't work because M5Stamp C3 is not yet implemented as an option or something. 
- DHT11 from keyestudio and the old dht11 library downloaded from their link.
**Doc :** https://wiki.keyestudio.com/Ks0034_keyestudio_DHT11_Temperature_and_Humidity_Sensor
**Lib :** https://fs.keyestudio.com/KS0034

For more I2C stuff, check [issue #2](https://github.com/nicolasdb/M5Stamp-C3U/issues/2)