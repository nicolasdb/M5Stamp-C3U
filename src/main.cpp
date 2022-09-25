#include <SPIFFS.h>
#include <SSVTimer.h>
#include <FS.h>

String dataMessage;


// SPIFFS functions
#pragma region        // shrink SPIFFS

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
  int sensor = analogRead(3);
  dataMessage = String(sensor) + "," + "\r\n";
    // Note: the “\r\n” at the end ensures the next reading is written on the next line.
  appendFile(SPIFFS, "/log.txt", dataMessage.c_str());
  Serial.println(sensor);
  }

// define timers
SSVTimer timer1;

// define sensor and button
#define LDR         3
#define BUTTON      9
int lastState = HIGH;
int currentState;  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LDR, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  // start SPIFFS
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  writeFile(SPIFFS, "/log.txt", "Reading ID, Date, Hour, Temperature \r\n");
    // Note: the “\r\n” at the end ensures the next reading is written on the next line.
  listDir(SPIFFS, "/", 0);
  
  timer1.SetEnabled(true);
  timer1.SetInterval(3*1000);
  timer1.SetOnTimer(probe);

  delay(500);
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
}