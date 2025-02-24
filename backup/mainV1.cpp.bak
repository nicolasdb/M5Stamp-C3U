#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <FastLED.h>
#include <Adafruit_PN532.h>
#include <ArduinoJson.h>
#define HTTPCLIENT_DISABLE_DEBUG
#include <HTTPClient.h>
#include "credentials.h"

// Pin Definitions
#define I2C_SDA 4
#define I2C_SCL 3
#define LED_PIN 2
#define NUM_LEDS 1

// NFC Constants
#define PN532_IRQ   -1
#define PN532_RESET -1

// LED Settings
#define LED_BRIGHTNESS 32  // 0-255, lower values for less intense light
#define COLOR_BLUE    CRGB(0, 0, 255)   // Waiting for tag
#define COLOR_GREEN   CRGB(0, 255, 0)   // Tag present
#define COLOR_RED     CRGB(255, 0, 0)   // Error state

// Debug Levels
#define DEBUG_NONE 0    // No debug output
#define DEBUG_NORMAL 1  // Only state changes and payloads
#define DEBUG_VERBOSE 2 // All debug information

#define CURRENT_DEBUG_LEVEL DEBUG_NORMAL  // Set default level

// State Machine States
enum NFCState {
    IDLE,
    TAG_DETECTED,
    TAG_PRESENT,
    TAG_REMOVED
};

// Timing Constants
#define HEALTH_CHECK_INTERVAL 30000   // 30 seconds for general health check
#define TAG_HEALTH_INTERVAL 60000     // 60 seconds for tag presence check
#define TAG_READ_INTERVAL 2000        // 2 seconds between tag reads
#define WEBHOOK_TIMEOUT 5000          // 5 seconds webhook timeout
#define I2C_RETRY_DELAY 100          // 100ms between I2C retries

// Global Variables
CRGB leds[NUM_LEDS];
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
String deviceId;
String currentTagId = "";
NFCState currentState = IDLE;
unsigned long lastHealthCheck = 0;
unsigned long lastTagCheck = 0;
unsigned long lastTagHealth = 0;

/**
 * Sets the LED color to indicate device status
 */
void setLedColor(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

/**
 * Generates or retrieves the device ID
 */
String getDeviceId() {
  const char* configuredName = DEVICE_NAME;
  if (configuredName && strlen(configuredName) > 0) {
    return String(configuredName);
  }
  
  uint64_t chipId = ESP.getEfuseMac();
  char deviceIdBuffer[13];
  snprintf(deviceIdBuffer, sizeof(deviceIdBuffer), "NFC_%06X", (uint32_t)chipId);
  return String(deviceIdBuffer);
}

/**
 * Initializes WiFi connection
 */
void setupWiFi() {
  if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
    Serial.printf("[INFO] Connecting to WiFi: %s\n", WIFI_SSID);
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  setLedColor(COLOR_RED);  // Indicate connecting state
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
    Serial.printf("\n[INFO] WiFi Connected. IP: %s\n", WiFi.localIP().toString().c_str());
  }
}

/**
 * Initializes NFC module
 */
void setupNFC() {
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("[ERROR] PN532 not found!");
    setLedColor(COLOR_RED);
    while (1); // Halt
  }

  if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
    Serial.printf("[INFO] PN532 Found - Firmware version: %d.%d\n", 
                  (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);
  }

  nfc.SAMConfig(); // Configure board to read RFID tags
}

/**
 * Generates JSON payload for webhook
 */
String getHealthMetrics(const String& tagId, const String& action) {
  StaticJsonDocument<512> doc;
  
  doc["event_type"] = tagId.isEmpty() ? "health_check" : "tag_event";
  doc["device_id"] = deviceId;
  doc["timestamp"] = String(millis()); // TODO: Add proper timestamp when RTC is available
  
  JsonObject tagStatus = doc.createNestedObject("tag_status");
  tagStatus["tag_id"] = tagId.isEmpty() ? "" : tagId;
  tagStatus["action"] = action.isEmpty() ? "" : action;
  
  JsonObject healthMetrics = doc.createNestedObject("health_metrics");
  healthMetrics["wifi_strength"] = WiFi.RSSI();
  healthMetrics["uptime"] = millis() / 1000;
  healthMetrics["free_memory"] = ESP.getFreeHeap();

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

/**
 * Sends data to webhook endpoint
 */
void sendWebhookData(const String& tagId, const String& action) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] WiFi not connected");
    setLedColor(COLOR_RED);
    return;
  }

  HTTPClient http;
  http.begin(WEBHOOK_URL);
  http.addHeader("Content-Type", "application/json");
  
  String jsonPayload = getHealthMetrics(tagId, action);
  if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
    Serial.printf("[INFO] Payload: %s\n", jsonPayload.c_str());
  }
  
  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode > 0) {
    if (CURRENT_DEBUG_LEVEL >= DEBUG_VERBOSE) {
      Serial.printf("[INFO] HTTP Response code: %d\n", httpResponseCode);
    }
  } else {
    Serial.printf("[ERROR] HTTP Error: %d\n", httpResponseCode);
    setLedColor(COLOR_RED);
  }
  
  http.end();
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
    Serial.println("\nNFC Time Tracker Starting...");
  }

  // Initialize LED with reduced brightness
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  setLedColor(COLOR_RED);  // Initial state: error/not ready

  // Get Device ID
  deviceId = getDeviceId();
  if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
    Serial.printf("[INFO] Device ID: %s\n", deviceId.c_str());
  }

  // Setup WiFi
  setupWiFi();

  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Setup NFC
  setupNFC();

  // Initial health check
  sendWebhookData("", "health_check");
  
  setLedColor(COLOR_BLUE);  // Ready state
}

/**
 * Print debug information about current state
 */
void printDebugInfo(uint8_t success, uint8_t* uid, uint8_t uidLength) {
    if (CURRENT_DEBUG_LEVEL >= DEBUG_VERBOSE) {
        Serial.println("\n[DEBUG] NFC Reader Status:");
        Serial.printf("  Current State: %s\n", 
            currentState == IDLE ? "IDLE" :
            currentState == TAG_DETECTED ? "TAG_DETECTED" :
            currentState == TAG_PRESENT ? "TAG_PRESENT" :
            currentState == TAG_REMOVED ? "TAG_REMOVED" : "UNKNOWN");
        Serial.printf("  Current Tag ID: %s\n", currentTagId.isEmpty() ? "NONE" : currentTagId.c_str());
        Serial.printf("  Read Success: %d\n", success);
        Serial.printf("  UID Length: %d\n", uidLength);
        
        if (success) {
            Serial.print("  Raw UID bytes: ");
            for (uint8_t i = 0; i < uidLength; i++) {
                Serial.printf("%02X ", uid[i]);
            }
            Serial.println();
        }
        
        // Print LED state
        CRGB currentColor = leds[0];
        String colorState = "UNKNOWN";
        if (currentColor == COLOR_BLUE) colorState = "BLUE (Waiting)";
        else if (currentColor == COLOR_GREEN) colorState = "GREEN (Tag Present)";
        else if (currentColor == COLOR_RED) colorState = "RED (Error)";
        Serial.printf("  LED State: %s\n", colorState.c_str());
        
        // Print timing info
        Serial.printf("  Time since last health check: %dms\n", millis() - lastHealthCheck);
        Serial.printf("  Time since last tag check: %dms\n", millis() - lastTagCheck);
        if (currentState == TAG_PRESENT) {
            Serial.printf("  Time since last tag health: %dms\n", millis() - lastTagHealth);
        }
    }
}

/**
 * Attempt to read an NFC tag
 * Returns true if tag was successfully read
 */
bool readNFCTag(String& tagId) {
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
        tagId = "";
        const char hexChars[] = "0123456789ABCDEF";
        for (uint8_t i = 0; i < uidLength; i++) {
            tagId += hexChars[(uid[i] >> 4) & 0x0F];
            tagId += hexChars[uid[i] & 0x0F];
        }
        return true;
    }
    return false;
}

void loop() {
    unsigned long currentTime = millis();
    String detectedTagId;
    
    // State machine
    switch (currentState) {
        case IDLE:
            // Regular device health check
            if (currentTime - lastHealthCheck >= HEALTH_CHECK_INTERVAL) {
                if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
                    Serial.println("[STATE] IDLE: Sending health check");
                }
                sendWebhookData("", "health_check");
                lastHealthCheck = currentTime;
            }
            
            // Check for new tag
            if (currentTime - lastTagCheck >= TAG_READ_INTERVAL) {
                if (readNFCTag(detectedTagId)) {
                    if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
                        Serial.printf("[STATE] IDLE -> TAG_DETECTED: Found tag %s\n", detectedTagId.c_str());
                    }
                    currentTagId = detectedTagId;
                    currentState = TAG_DETECTED;
                }
                lastTagCheck = currentTime;
            }
            break;
            
        case TAG_DETECTED:
            // Send webhook for new tag
            if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
                Serial.printf("[STATE] TAG_DETECTED: Sending placed event for tag %s\n", currentTagId.c_str());
            }
            setLedColor(COLOR_GREEN);
            sendWebhookData(currentTagId, "placed");
            lastTagHealth = currentTime;  // Initialize tag health check timer
            currentState = TAG_PRESENT;
            break;
            
        case TAG_PRESENT:
            // Check if tag is still there
            if (currentTime - lastTagCheck >= TAG_READ_INTERVAL) {
                if (!readNFCTag(detectedTagId)) {
                    if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
                        Serial.println("[STATE] TAG_PRESENT -> TAG_REMOVED: Tag lost");
                    }
                    currentState = TAG_REMOVED;
                }
                lastTagCheck = currentTime;
            }
            
            // Send tag health check
            if (currentTime - lastTagHealth >= TAG_HEALTH_INTERVAL) {
                if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
                    Serial.printf("[STATE] TAG_PRESENT: Sending still_present for tag %s\n", currentTagId.c_str());
                }
                sendWebhookData(currentTagId, "still_present");
                lastTagHealth = currentTime;
            }
            break;
            
        case TAG_REMOVED:
            if (CURRENT_DEBUG_LEVEL >= DEBUG_NORMAL) {
                Serial.printf("[STATE] TAG_REMOVED: Sending removed event for tag %s\n", currentTagId.c_str());
            }
            setLedColor(COLOR_BLUE);
            sendWebhookData(currentTagId, "removed");
            currentTagId = "";
            currentState = IDLE;
            break;
    }
    
    delay(50);  // Small delay to prevent I2C bus overload
}
