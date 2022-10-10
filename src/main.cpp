#include <Arduino.h>
#include <Wire.h>                     // controle I2C
#include <Adafruit_MCP23X17.h>        // controle MCP
#include <AccelStepper.h>
#include <MCP3017AccelStepper.h>
 
#define I2C_Freq 100000UL             // Standard-Mode (Sm), with a bit rate up to 100 kbit/s
 
#define SDA_1 1                       // I choose these pins for I2C.
#define SCL_1 0
 
Adafruit_MCP23X17 mcp1;        

int LED_PIN[] = {0,1,2,3};                // MCP23017 pin LED
int SWPins[] = {4,5,6,7};              // MCP23017 pin "Switchboard"

bool state;

#define STEPPER_COUNT 1
MCP3017AccelStepper steppers[STEPPER_COUNT] = {
    // interface, step, dir, en
    MCP3017AccelStepper(AccelStepper::DRIVER, 15, 14, 8),
    // MCP3017AccelStepper(AccelStepper::DRIVER, 4, 5, 6),
    // MCP3017AccelStepper(AccelStepper::DRIVER, 12, 11, 10),
    // MCP3017AccelStepper(AccelStepper::DRIVER, 3, 1, 2),
};
 
void setup() {
  Wire.begin(SDA_1 , SCL_1 , I2C_Freq);
  mcp.begin_I2C();
  Serial.begin(115200);

  for (int i = 0; i < STEPPER_COUNT; i++) {
    steppers[i].setMcp(mcp1);
    steppers[i].enableOutputs();
    steppers[i].setMaxSpeed(150.0);
    steppers[i].setAcceleration(100.0);
    steppers[i].moveTo(200);
  }


  for (int i = 0; i < 4 ; i++){     // set pin 0>3 as Output for LED
    mcp1.pinMode(LED_PIN[i], OUTPUT);
    mcp1.digitalWrite(i, LOW);
    delay(50);
  }
  for (int i = 0; i < 4 ; i++){     // set pin 4>7 as Input for switchboard
    mcp1.pinMode(SWPins[i], INPUT_PULLUP);
    delay(50);
  }
  
  // Unlike pinMode(INPUT), there is no pull-down resistor necessary with INPUT_PULLUP. An internal
  // 20K-ohm resistor is pulled to 5V. This configuration causes the input to read HIGH 
  // when the switch is open, and LOW when it is closed.

}
                 
void loop() {

  for (byte i = 0; i < 4; i = i + 1) {
    state = mcp1.digitalRead(SWPins[i]);
     if (state == 0)
      {
        mcp1.digitalWrite(LED_PIN[i],HIGH);
      }   
      else {
        mcp1.digitalWrite(LED_PIN[i],LOW);
      }      
    Serial.print(state);
  }

  if (steppers[0].distanceToGo() == 0) {
    steppers[0].moveTo(-steppers[0].currentPosition());
  }

  for (int i = 0; i < STEPPER_COUNT; i++) {
    steppers[i].run();
  }

Serial.println();                      // just to insert a new line after every loop

delay(1000);                           // wait for a second


}
