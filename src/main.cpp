#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "wifi_manager.h"
#include "webhook_manager.h"

// Initialize PN532 using I2C (with IRQ and RESET pins disabled)
Adafruit_PN532 nfc(-1, -1);  // Use I2C, disable IRQ and RESET pins

// Initialize NeoPixel
Adafruit_NeoPixel pixels(1, BUILTIN_LED_PIN, NEO_GRB + NEO_KHZ800);

// Initialize managers
WiFiManager wifiManager;
WebhookManager webhookManager;

// Store last successful tag read
String lastTagId = "";

// Timing management
unsigned long lastReadTime = 0;
unsigned long lastWifiCheckTime = 0;
const int I2C_RECOVERY_TIME = 100;  // ms between I2C operations
const int TAG_READ_TIMEOUT = 100;   // ms (increased for better detection)
const int MAX_UID_LENGTH = 7;       // Maximum UID length we'll handle
const int WIFI_CHECK_INTERVAL = 5000; // Check WiFi every 5 seconds

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

void initializeRFID() {
    // Increase I2C clock speed for more reliable communication
    Wire.setClock(100000); // 100kHz standard mode
    
    // Initialize I2C for ESP32-C3
    Wire.begin(PN532_SDA, PN532_SCL);
    
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
}

void setup() {
    // Initialize Serial for debugging
    DEBUG_SERIAL.begin(SERIAL_BAUD);
    DEBUG_SERIAL.println("\nRFID Switch - Phase 2");
    DEBUG_SERIAL.println("Initializing...");
    
    // Initialize NeoPixel
    pixels.begin();
    pixels.setBrightness(LED_BRIGHTNESS);
    pixels.setPixelColor(0, COLOR_WIFI_CONNECTING);
    pixels.show();
    
    // Initialize RFID
    initializeRFID();
    
    // Initialize and connect WiFi
    if (!wifiManager.begin()) {
        DEBUG_SERIAL.println("Failed to initialize WiFi!");
        while (1) {
            pixels.setPixelColor(0, COLOR_ERROR);
            pixels.show();
            delay(LED_ERROR_BLINK_INTERVAL);
            pixels.setPixelColor(0, 0);
            pixels.show();
            delay(LED_ERROR_BLINK_INTERVAL);
        }
    }

    // Initialize webhook manager
    if (!webhookManager.begin()) {
        DEBUG_SERIAL.println("Failed to initialize webhook manager!");
    }
    
    DEBUG_SERIAL.println("Setup complete!");
    DEBUG_SERIAL.println("-------------------------");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check WiFi connection periodically
    if (currentTime - lastWifiCheckTime >= WIFI_CHECK_INTERVAL) {
        lastWifiCheckTime = currentTime;
        wifiManager.checkConnection();
    }
    
    // Enforce consistent RFID polling interval
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
    lastReadTime = currentTime;
    
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
        currentUid.trim(); // Remove trailing whitespace
    }
    
    // Print debug only on state change or new UID
    if (success != lastSuccess || (success && currentUid != lastUid)) {
        // Update lastTagId on successful read
        if (success) {
            lastTagId = currentUid;
        }

        DEBUG_SERIAL.println("\nRFID Poll Result:");
        DEBUG_SERIAL.printf("Timestamp: %s\n", wifiManager.getFormattedTime().c_str());
        DEBUG_SERIAL.printf("Tag Present: %s\n", success ? "YES" : "NO");
        DEBUG_SERIAL.printf("Tag ID: %s\n", success ? currentUid.c_str() : lastTagId.c_str());
        
        if (success) {
            String tagType = currentTagType == MIFARE_CLASSIC ? "Mifare Classic (4-byte)" :
                           currentTagType == ISO14443_4 ? "ISO14443-4 (7-byte)" : "Unknown";
            DEBUG_SERIAL.printf("Tag Type: %s\n", tagType.c_str());
            
            // Print WiFi and time sync status
            String wifiStatus;
            String timeStatus;
            if (wifiManager.isConnected()) {
                wifiStatus = String("Connected to ") + wifiManager.getCurrentSSID() + 
                           String(" (") + String(wifiManager.getRSSI()) + String(" dBm)");
                timeStatus = wifiManager.isTimeSynced() ? "Synced with NTP" : "Not synced";
                DEBUG_SERIAL.printf("WiFi: %s\n", wifiStatus.c_str());
                DEBUG_SERIAL.printf("Time_Status: %s\n", timeStatus.c_str());
            } else {
                DEBUG_SERIAL.println("WiFi: Disconnected");
                DEBUG_SERIAL.println("Time_Status: Not synced (No WiFi)");
            }
        }

        // Send webhook if WiFi connected (for both tag insert and removal)
        if (wifiManager.isConnected()) {
            String wifiStatus = String("Connected to ") + wifiManager.getCurrentSSID() + 
                              String(" (") + String(wifiManager.getRSSI()) + String(" dBm)");
            String timeStatus = wifiManager.isTimeSynced() ? "Synced with NTP" : "Not synced";
            String tagType = success ? (currentTagType == MIFARE_CLASSIC ? "Mifare Classic (4-byte)" :
                                      currentTagType == ISO14443_4 ? "ISO14443-4 (7-byte)" : "Unknown") : "";
            
            webhookManager.sendPollResult(
                success,
                currentUid,
                lastTagId,
                tagType,
                wifiStatus,
                timeStatus,
                wifiManager.getFormattedTime()
            );
        }
    }
    
    // Update LED state based on WiFi and tag status
    if (success && currentTagType != UNKNOWN) {
        pixels.setPixelColor(0, COLOR_TAG_PRESENT);  // Green when tag present
    } else if (wifiManager.isConnected()) {
        pixels.setPixelColor(0, COLOR_WIFI_CONNECTED);  // Blue when WiFi connected
    } else {
        pixels.setPixelColor(0, COLOR_WIFI_CONNECTING);  // Blinking blue when connecting
    }
    pixels.show();
    
    // Store state for next comparison
    lastSuccess = success;
    lastUid = currentUid;
    lastTagType = currentTagType;
}
