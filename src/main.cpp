#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

// Initialize PN532 using I2C (with IRQ and RESET pins disabled)
Adafruit_PN532 nfc(-1, -1);  // Use I2C, disable IRQ and RESET pins

// Initialize NeoPixel
Adafruit_NeoPixel pixels(1, BUILTIN_LED_PIN, NEO_GRB + NEO_KHZ800);

// Timing management
unsigned long lastReadTime = 0;
const int I2C_RECOVERY_TIME = 100;  // ms between I2C operations
const int TAG_READ_TIMEOUT = 100;   // ms (increased for better detection)
const int MAX_UID_LENGTH = 7;       // Maximum UID length we'll handle

// Tag type detection
enum TagType {
    UNKNOWN,
    MIFARE_CLASSIC,    // 4-byte UID
    ISO14443_4         // 7-byte UID
};

TagType detectTagType(uint8_t uidLength) {
    switch (uidLength) {
        case 4:
            return MIFARE_CLASSIC;
        case 7:
            return ISO14443_4;
        default:
            return UNKNOWN;
    }
}

void setup() {
  // Increase I2C clock speed for more reliable communication
  Wire.setClock(100000); // 100kHz standard mode
  
  DEBUG_SERIAL.begin(SERIAL_BAUD);
  DEBUG_SERIAL.println("\nRFID Switch - Phase 1");
  DEBUG_SERIAL.println("Initializing...");
  
  // Initialize I2C for ESP32-C3
  Wire.begin(PN532_SDA, PN532_SCL);
  
  // Initialize NeoPixel
  pixels.begin();
  pixels.setBrightness(LED_BRIGHTNESS);
  pixels.setPixelColor(0, COLOR_WIFI_CONNECTING); // Blue while initializing
  pixels.show();
  
  // Initialize PN532
  if (!nfc.begin()) {
    DEBUG_SERIAL.println("Error: Could not initialize PN532!");
    while (1) {
      pixels.setPixelColor(0, COLOR_ERROR);
      pixels.show();
      delay(LED_ERROR_BLINK_INTERVAL);
      pixels.setPixelColor(0, 0);
      pixels.show();
      delay(LED_ERROR_BLINK_INTERVAL);
    }
  }
  
  // Check for PN532 board
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    DEBUG_SERIAL.println("Error: Did not find PN532 board!");
    while (1) {
      pixels.setPixelColor(0, COLOR_ERROR);
      pixels.show();
      delay(LED_ERROR_BLINK_INTERVAL);
      pixels.setPixelColor(0, 0);
      pixels.show();
      delay(LED_ERROR_BLINK_INTERVAL);
    }
  }
  
  // Print firmware version
  DEBUG_SERIAL.print("Found chip PN5"); 
  DEBUG_SERIAL.println((versiondata>>24) & 0xFF, HEX);
  DEBUG_SERIAL.print("Firmware ver. ");
  DEBUG_SERIAL.print((versiondata>>16) & 0xFF, DEC);
  DEBUG_SERIAL.print('.'); 
  DEBUG_SERIAL.println((versiondata>>8) & 0xFF, DEC);
  
  // Configure PN532
  nfc.SAMConfig();
  
  // Set initial color
  pixels.setPixelColor(0, COLOR_WIFI_CONNECTING); // Blue for ready state
  pixels.show();
  
  DEBUG_SERIAL.println("Ready to read RFID tags!");
  DEBUG_SERIAL.println("-------------------------");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Enforce consistent polling interval
  if (currentTime - lastReadTime < LED_SLOW_BLINK_INTERVAL) {
    delay(10); // Small yield delay
    return;
  }

  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  bool success = false;
  
  // Add small delay between I2C operations
  delay(I2C_RECOVERY_TIME);
  
  // First try ISO14443A detection
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, TAG_READ_TIMEOUT);
  
  // Update timing after I2C operation
  lastReadTime = millis();
  
  static bool lastSuccess = false;
  static String lastUid = "";
  static TagType lastTagType = UNKNOWN;
  String currentUid = "";
  TagType currentTagType = UNKNOWN;
  
  if (success) {
    currentTagType = detectTagType(uidLength);
    
    // Build current UID string
    for (uint8_t i = 0; i < uidLength && i < MAX_UID_LENGTH; i++) {
      if (uid[i] < 0x10) currentUid += "0";
      currentUid += String(uid[i], HEX);
      currentUid += " ";
    }
  }
  
  // Print debug only on state change or new UID
  if (success != lastSuccess || (success && currentUid != lastUid)) {
    DEBUG_SERIAL.println("\nRFID Poll Result:");
    DEBUG_SERIAL.printf("Tag Present: %s\n", success ? "YES" : "NO");
    
    if (success) {
      DEBUG_SERIAL.printf("Tag ID: %s\n", currentUid.c_str());
      DEBUG_SERIAL.printf("Tag Type: %s\n", 
        currentTagType == MIFARE_CLASSIC ? "Mifare Classic (4-byte)" :
        currentTagType == ISO14443_4 ? "ISO14443-4 (7-byte)" : "Unknown");
    }
  }
  
  // Update LED state based on valid read
  if (success && currentTagType != UNKNOWN) {
    pixels.setPixelColor(0, COLOR_TAG_PRESENT);
  } else {
    pixels.setPixelColor(0, COLOR_WIFI_CONNECTING);
  }
  pixels.show();
  
  // Store state for next comparison
  lastSuccess = success;
  lastUid = currentUid;
  lastTagType = currentTagType;
}
