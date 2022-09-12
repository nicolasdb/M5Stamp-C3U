//
//    FILE: AM2320_test.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo sketch for AM2320 I2C humidity & temperature sensor
//
//  AM232X PIN layout             AM2315 COLOR
//  ============================================
//   bottom view  DESCRIPTION     COLOR
//       +---+
//       |o  |       VDD          RED
//       |o  |       SDA          YELLOW
//       |o  |       GND          BLACK
//       |o  |       SCL          GREY
//       +---+
//
// do not forget pull up resistors between SDA, SCL and VDD.

#include "Arduino.h"
#include "Wire.h"
#include "AM232X.h"

AM232X AM2320;


void setup()
{
  Serial.begin(115200);
  Wire.begin(6,7);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(AM232X_LIB_VERSION);
  Serial.println();

  if (! AM2320.begin() )
  {
    Serial.println("Sensor not found");
    while (1);
  }
  AM2320.wakeUp();
  delay(2000);

  // sensor only returns one decimal.
  Serial.println();
  Serial.println("Read:\t");

  int status = AM2320.read();
  switch (status)
  {
    case AM232X_OK:
      Serial.println("OK");
      break;
    default:
      Serial.println(status);
      break;
  }


  Serial.println("done...\n");
}


void loop()
{
  delay(2000);

  int status = AM2320.read();
  switch (status)
  {
    case AM232X_OK:
      Serial.println("OK");
      break;
    default:
      Serial.println(status);
      break;
  }
  Serial.print("Temp:\t");
  Serial.println(AM2320.getTemperature(), 1);
  Serial.print("Hum:\t");
  Serial.println(AM2320.getHumidity(), 1);
}


// -- END OF FILE --