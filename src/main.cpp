#include <SPIFFS.h>
#include <SSVTimer.h>
#include <FS.h>

#include <Wire.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h>
#include <AM232X.h>

#define SCREEN_WIDTH 128            // OLED display width, in pixels
#define SCREEN_HEIGHT 64            // OLED display height, in pixels
#define I2C_SDA 1                   // I2C
#define I2C_SCL 0                   // I2C

// define variables
#define sensorPin   3
int sensor;
float temp;
float hum;
String dataMessage;                 // to collect data before saving to log
#define BUTTON      9
int lastState = HIGH;
int currentState;  

// define timers
SSVTimer timer1;

// define sensor
AM232X AM2320;

// SPIFFS functions
#pragma region        // to shrink SPIFFS lines in VS code

#define FORMAT_SPIFFS_IF_FAILED true

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}
#pragma endregion


// read sensor value function
void probe() {
  sensor = analogRead(sensorPin);
  temp = AM2320.getTemperature();
  hum = AM2320.getHumidity();
  Serial.print(sensor);
  Serial.print(" , Temp: ");
  Serial.print(temp); Serial.print("°C");
  Serial.print(" , ");
  Serial.print(hum,1); Serial.println("%");

  dataMessage = String(sensor) + "," + String(temp) + "," + String(hum) + "\r\n";
    // Note: the “\r\n” at the end ensures the next reading is written on the next line.
  appendFile(SPIFFS, "/log.txt", dataMessage.c_str());
  }

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA,I2C_SCL);
  AM2320.begin();
  pinMode(sensorPin, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  // start SPIFFS
  if(!SPIFFS.begin()){
 // if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){                         // to format partition
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  //writeFile(SPIFFS, "/log.txt", "Reading LDR, Temperature \r\n");     // to create the file 
    // Note: the “\r\n” at the end ensures the next reading is written on the next line.
  listDir(SPIFFS, "/", 0);
  
  timer1.SetEnabled(true);
  timer1.SetInterval((5)*1000);
  timer1.SetOnTimer(probe);

   if (! AM2320.begin() )
  {
    Serial.println("Sensor not found");
    while (1);
  }
  AM2320.wakeUp();
  delay(2000);

  int status = AM2320.read();
  switch (status)
  {
    case AM232X_OK:
      Serial.println("AM2320 sensor OK");
      break;
    default:
      Serial.println(status);
      break;
  }

  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  timer1.RefreshIt();
  currentState = digitalRead(BUTTON);
      if(lastState == LOW && currentState == HIGH) {
        Serial.println("Button Pressed!");
        readFile(SPIFFS, "/log.txt");
      }
  lastState = currentState;
  delay(100);
}