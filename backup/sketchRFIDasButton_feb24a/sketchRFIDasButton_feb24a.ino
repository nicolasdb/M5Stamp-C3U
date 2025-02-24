#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>

// Define pins for PN532
#define PN532_SCL 3
#define PN532_SDA 4

// Define pin for NeoPixel
#define NEOPIXEL_PIN 2
#define NUM_PIXELS 1

// Initialize PN532 using I2C
Adafruit_PN532 nfc(PN532_SDA, PN532_SCL);

// Initialize NeoPixel
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Colors
const uint32_t BLUE = pixels.Color(0, 0, 255);   // Initial color
const uint32_t GREEN = pixels.Color(0, 255, 0);  // Active color

void setup() {
  Serial.begin(115200);
  
  // Initialize NeoPixel
  pixels.begin();
  pixels.setBrightness(50);  // Set brightness to 50%
  pixels.show();
  
  // Initialize PN532
  nfc.begin();
  
  // Configure PN532
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 not found!");
    while (1) {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // Red to indicate error
      pixels.show();
      delay(500);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));    // Off
      pixels.show();
      delay(500);
    }
  }
  
  // Set up PN532
  nfc.SAMConfig();
  
  // Set initial color
  pixels.setPixelColor(0, BLUE);
  pixels.show();
  
  Serial.println("Ready to read RFID tags!");
}

void loop() {
  boolean tagPresent = false;
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                         // Length of the UID
  
  // Check for presence of RFID tag
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    tagPresent = true;
    pixels.setPixelColor(0, GREEN);
    pixels.show();
  } else {
    tagPresent = false;
    pixels.setPixelColor(0, BLUE);
    pixels.show();
  }
  
  delay(100);  // Small delay to prevent too frequent readings
}