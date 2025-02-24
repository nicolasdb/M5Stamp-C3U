#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>

// Define pins
#define PN532_SDA 4
#define PN532_SCL 3
#define NEOPIXEL_PIN 2
#define NUM_PIXELS 1

// Initialize PN532 using I2C (with IRQ and RESET pins disabled)
Adafruit_PN532 nfc(-1, -1);  // Use I2C, disable IRQ and RESET pins

// Initialize NeoPixel
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Colors
const uint32_t BLUE = pixels.Color(0, 0, 255);   // Initial color
const uint32_t GREEN = pixels.Color(0, 255, 0);  // Active color

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C for ESP32-C3
  Wire.begin(PN532_SDA, PN532_SCL);
  
  // Initialize NeoPixel
  pixels.begin();
  pixels.setBrightness(50);  // Set brightness to 50%
  pixels.show();
  
  // Initialize PN532
  if (!nfc.begin()) {
    Serial.println("Error: Could not initialize PN532!");
    while (1) {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // Red to indicate error
      pixels.show();
      delay(500);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));    // Off
      pixels.show();
      delay(500);
    }
  }
  
  // Check for PN532 board
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Error: Did not find PN532 board!");
    while (1) {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // Red to indicate error
      pixels.show();
      delay(500);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));    // Off
      pixels.show();
      delay(500);
    }
  }
  
  // Print firmware version
  Serial.print("Found chip PN5"); 
  Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); 
  Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Configure PN532
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
  
  // Check for presence of RFID tag with 50ms timeout
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);
  
  // Print raw values
  Serial.print("Raw - Success: ");
  Serial.print((int)success);
  Serial.print(", TagPresent: ");
  Serial.print((int)tagPresent);
  Serial.print(", UID Length: ");
  Serial.print(uidLength);
  Serial.print(", UID: ");
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(uid[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  if (success) {
    tagPresent = true;
    pixels.setPixelColor(0, GREEN);
    pixels.show();
  } else {
    tagPresent = false;
    pixels.setPixelColor(0, BLUE);
    pixels.show();
  }
  
  delay(1000);  // Small delay to prevent too frequent readings
}
