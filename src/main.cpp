/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include "Arduino.h"
#include <WiFi.h>
#include <Wire.h>                    // controle I2C
#include <Adafruit_MCP23X17.h>       // controle MCP

#define I2C_Freq 100000UL 
#define SDA_1 1                      
#define SCL_1 0
Adafruit_MCP23X17 mcp;

// Replace with your network credentials
const char* ssid = "EAsy-Access-Point";
const char* password = "1234";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputmcp0State = "off";
String outputmcp1State = "off";

// Assign output variables to GPIO pins
const int outputmcp0 = 0;
const int outputmcp1 = 1;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_1 , SCL_1 , I2C_Freq);
  mcp.begin_I2C();
  // Initialize the output variables as outputs
  mcp.pinMode(outputmcp0, OUTPUT);
  mcp.pinMode(outputmcp1, OUTPUT);
  // Set outputs to LOW
  mcp.digitalWrite(outputmcp0, LOW);
  mcp.digitalWrite(outputmcp1, LOW);
    delay(2000);                                // adding a delay, monitor need a bit more time to connect

  // Connect to Wi-Fi network with SSID and password
  Serial.println("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  //WiFi.softAP(ssid, password);
  WiFi.softAP(ssid);
  
  // Print AccessPoint IP address and start web server
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Page on this IP address: ");
  Serial.println(IP);

  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /0/on") >= 0) {
              Serial.println("mcpGPIO 0 on");
              outputmcp0State = "on";
              mcp.digitalWrite(outputmcp0, HIGH);
            } else if (header.indexOf("GET /0/off") >= 0) {
              Serial.println("mcpGPIO 0 off");
              outputmcp0State = "off";
              mcp.digitalWrite(outputmcp0, LOW);
            } else if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("mcpGPIO 1 on");
              outputmcp1State = "on";
              mcp.digitalWrite(outputmcp1, HIGH);
            } else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("mcpGPIO 1 off");
              outputmcp1State = "off";
              mcp.digitalWrite(outputmcp1, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for MCP GPIO 0  
            client.println("<p>MCP GPIO 0 - State " + outputmcp0State + "</p>");
            // If the outputmcp0State is off, it displays the ON button       
            if (outputmcp0State=="off") {
              client.println("<p><a href=\"/0/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/0/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for MCP GPIO 1  
            client.println("<p>MCP GPIO 1 - State " + outputmcp1State + "</p>");
            // If the outputmcp1State is off, it displays the ON button       
            if (outputmcp1State=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}