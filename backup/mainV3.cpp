#include <Arduino.h>
#include "config.h"
#include "WiFiManager.h"
#include "TimeManager.h"
#include "StorageManager.h"
#include "WebhookManager.h"
#include "RFIDManager.h"

// System components
WiFiManager wifi;
TimeManager ntp;
StorageManager spiffs;
WebhookManager webhook;
RFIDManager rfid;

// Function declarations
bool initializeSystem();
void handleTagStateChange();
void logAndSendEvent(const String& tagId, const char* eventType);

void setup() {
    // Initialize serial communication
    DEBUG_SERIAL.begin(SERIAL_BAUD);
    DEBUG_SERIAL.println("\nStarting M5Stamp-C3U Time Tracker V3...");

    if (!initializeSystem()) {
        ESP.restart();
    }
}

void loop() {
    static bool lastWifiState = false;
    bool currentWifiState = wifi.connected();
    
    // Always update RFID first
    rfid.update();
    
    // Handle state changes if any
    if (rfid.hasStateChanged()) {
        handleTagStateChange();
    }
    
    // Handle WiFi - don't return early
    if (!currentWifiState) {
        wifi.reconnect();
    }
    
    // Log WiFi state changes
    if (lastWifiState != currentWifiState) {
        DEBUG_SERIAL.printf("\nWiFi State Change: %s\n", currentWifiState ? "Connected" : "Disconnected");
        lastWifiState = currentWifiState;
    }
    
    delay(50);  // Small delay for stability
}

bool initializeSystem() {
    // Setup SPIFFS
    if (!spiffs.begin()) {
        DEBUG_SERIAL.println("Failed to initialize SPIFFS!");
        return false;
    }

    // Initialize RFID
    if (!rfid.begin()) {
        DEBUG_SERIAL.println("Failed to initialize RFID reader!");
        return false;
    }
    DEBUG_SERIAL.println("RFID reader initialized successfully!");

    // Connect to WiFi
    if (!wifi.begin()) {
        DEBUG_SERIAL.println("Failed to connect to WiFi!");
        return false;
    }

    // Sync time
    if (!ntp.begin()) {
        DEBUG_SERIAL.println("Failed to sync time!");
        return false;
    }
    
    DEBUG_SERIAL.println("System ready. Waiting for tags...");
    return true;
}

void handleTagStateChange() {
    String tagId = rfid.getCurrentTagId();
    bool isPresent = rfid.isTagPresent();
    
    if (tagId.length() > 0) {  // Only process if we have a valid tag ID
        logAndSendEvent(tagId, isPresent ? "IN" : "OUT");
        
        // Clear stored tag ID after OUT event is processed
        if (!isPresent) {
            rfid.clearStoredTagId();
            DEBUG_SERIAL.println("Cleared stored tag ID after OUT event");
        }
    } else {
        DEBUG_SERIAL.println("Error: No valid tag ID for event");
    }
}

void logAndSendEvent(const String& tagId, const char* eventType) {
    String timestamp = ntp.getFormattedTime();
    if (timestamp.length() > 0) {
        DEBUG_SERIAL.printf("\nProcessing %s event for tag: %s\n", eventType, tagId.c_str());
        
        String recentLogs = spiffs.getRecentEntries(webhook.getEntriesToInclude());
        bool sent = webhook.sendPayload(tagId, eventType, timestamp, recentLogs);
        
        if (!sent) {
            DEBUG_SERIAL.println("Failed to send payload");
        } else {
            DEBUG_SERIAL.printf("Successfully sent %s event\n", eventType);
        }
        
        // Log the event regardless of webhook success
        bool logged = spiffs.writeLogEntry(timestamp.c_str(), tagId.c_str(), eventType, sent);
        if (!logged) {
            DEBUG_SERIAL.println("Failed to log event to SPIFFS");
        } else {
            DEBUG_SERIAL.println("Event logged successfully");
        }
    } else {
        DEBUG_SERIAL.println("Error: Invalid timestamp");
    }
}
