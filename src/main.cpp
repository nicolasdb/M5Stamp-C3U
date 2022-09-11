#include <Arduino.h>
#include <Wire.h>                     // controle I2C
#include <Adafruit_MCP23X17.h>        // controle MCP
 
#define I2C_Freq 100000UL             // Standard-Mode (Sm), with a bit rate up to 100 kbit/s
 
#define SDA_1 6                       // I choose these pins for I2C.
#define SCL_1 7
 
Adafruit_MCP23X17 mcp; 
         

int LED_PIN[] = {0,1,2,3};                // MCP23017 pin LED
int SWPins[] = {4,5,6,7};              // MCP23017 pin "Switchboard"

bool state;
 
void setup() {
  Wire.begin(SDA_1 , SCL_1 , I2C_Freq);
  mcp.begin_I2C();
  Serial.begin(115200);


  for (int i = 0; i < 4 ; i++){     // set pin 0>3 as Output for LED
    mcp.pinMode(LED_PIN[i], OUTPUT);
    mcp.digitalWrite(i, LOW);
    delay(50);
  }
  for (int i = 0; i < 4 ; i++){     // set pin 4>7 as Input for switchboard
    mcp.pinMode(SWPins[i], INPUT_PULLUP);
    delay(50);
  }
  
  // Unlike pinMode(INPUT), there is no pull-down resistor necessary with INPUT_PULLUP. An internal
  // 20K-ohm resistor is pulled to 5V. This configuration causes the input to read HIGH 
  // when the switch is open, and LOW when it is closed.

}
                 
void loop() {

  for (byte i = 0; i < 4; i = i + 1) {
    state = mcp.digitalRead(SWPins[i]);
     if (state == 0)
      {
        mcp.digitalWrite(LED_PIN[i],HIGH);
      }   
      else {
        mcp.digitalWrite(LED_PIN[i],LOW);
      }      
    Serial.print(state);
  }

Serial.println();                      // just to insert a new line after every loop

delay(1000);                           // wait for a second


}
