## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### M5Stamp C3U as an ACCESS POINT
`main.cpp` : 
The sketch will set the ESP32C3 as an access point, then a webserver is use to control ON/OFF two leds connected on the MCP23017 on my setup.  


> For ARDUINO IDE: You can copy/paste the code from `main.cpp` Don't forget to install libraries. 

#### Notes:
- Work with any web servers application, the important part is to replace this:
```
// Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
```

with this, to enable soft Access Point
```  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);
  
  // Print AccessPoint IP address and start web server
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Page on this IP address: ");
  Serial.println(IP);
```

- PlateformIO need time to start monitor. Add enough delay in `void setup ()` before the first `Serial.print()`  
  I didn't had the problem in Arduino IDE.  


Check [issue #9](https://github.com/nicolasdb/M5Stamp-C3U/issues/9)