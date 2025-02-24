#ifndef RFIDMANAGER_H
#define RFIDMANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "config.h"

class RFIDManager {
private:
    Adafruit_PN532* nfc;
    bool tagPresent;         // Current state
    bool previousTagPresent; // Previous state
    String storedTagId;      // Buffer for tag ID
    unsigned long lastReadTime;
    uint8_t lastUid[7];     // Persistent buffer for last read UID
    uint8_t lastUidLength;  // Length of last read UID
    
public:
    RFIDManager() : 
        lastReadTime(0), 
        previousTagPresent(false),
        tagPresent(false),
        storedTagId(""),
        lastUidLength(0) {
        memset(lastUid, 0, sizeof(lastUid));
        Wire.begin(PN532_SDA, PN532_SCL);
    }

    bool begin() {
        nfc = new Adafruit_PN532(-1, -1);  // Use I2C with default pins
        
        if (!nfc->begin()) {
            DEBUG_SERIAL.println("NFC Error: Could not initialize");
            return false;
        }
        
        uint32_t versiondata = nfc->getFirmwareVersion();
        if (!versiondata) {
            DEBUG_SERIAL.println("NFC Error: Did not find PN532 board");
            return false;
        }
        
        DEBUG_SERIAL.print("Found chip PN5");
        DEBUG_SERIAL.println((versiondata>>24) & 0xFF, HEX);
        DEBUG_SERIAL.print("Firmware ver. ");
        DEBUG_SERIAL.print((versiondata>>16) & 0xFF, DEC);
        DEBUG_SERIAL.print('.'); 
        DEBUG_SERIAL.println((versiondata>>8) & 0xFF, DEC);
        
        // Configure board to read RFID tags
        nfc->SAMConfig();
        
        return true;
    }

    void update() {
        previousTagPresent = tagPresent;
        
        // Always try to read
        uint8_t uid[7];
        uint8_t uidLength;
        bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        
        // Only update state if enough time has passed
        if (millis() - lastReadTime >= 1000) {
            lastReadTime = millis();
            
            // Update tag presence state
            tagPresent = success;
            
            if (success) {
                // Store UID in persistent buffer
                memcpy(lastUid, uid, uidLength);
                lastUidLength = uidLength;
                
                // Handle new tag detection
                if (!previousTagPresent) {
                    storedTagId = formatTagId(lastUid, lastUidLength);
                    DEBUG_SERIAL.println("\nTag IN detected");
                    DEBUG_SERIAL.printf("Stored Tag ID: %s\n", storedTagId.c_str());
                    printTagInfo(lastUid, lastUidLength);
                }
            } else if (previousTagPresent) {
                // Tag was removed
                DEBUG_SERIAL.println("\nTag OUT detected");
                DEBUG_SERIAL.printf("Using stored Tag ID: %s\n", storedTagId.c_str());
            }
            
            // Debug state changes
            if (previousTagPresent != tagPresent) {
                DEBUG_SERIAL.println("State Change Summary:");
                DEBUG_SERIAL.printf("Previous State: %s\n", previousTagPresent ? "Present" : "Not Present");
                DEBUG_SERIAL.printf("Current State: %s\n", tagPresent ? "Present" : "Not Present");
                DEBUG_SERIAL.printf("Event Type: %s\n", tagPresent ? "IN" : "OUT");
            }
        }
    }

    bool hasStateChanged() {
        return previousTagPresent != tagPresent;
    }

    bool isTagPresent() const {
        return tagPresent;
    }

    String getCurrentTagId() {
        return storedTagId;
    }

    void clearStoredTagId() {
        storedTagId = "";
        memset(lastUid, 0, sizeof(lastUid));
        lastUidLength = 0;
    }

private:
    String formatTagId(uint8_t* uid, uint8_t uidLength) {
        String rawId = "";
        for (int i = 0; i < uidLength; i++) {
            if (uid[i] < 0x10) {
                rawId += "0";
            }
            rawId += String(uid[i], HEX);
        }
        return rawId;
    }

    void printTagInfo(uint8_t* uid, uint8_t uidLength) {
        DEBUG_SERIAL.println("Tag detected!");
        DEBUG_SERIAL.print("UID Length: ");
        DEBUG_SERIAL.print(uidLength, DEC);
        DEBUG_SERIAL.println(" bytes");
        DEBUG_SERIAL.print("UID Value: ");
        
        for (uint8_t i = 0; i < uidLength; i++) {
            if (uid[i] < 0x10) {
                DEBUG_SERIAL.print("0");
            }
            DEBUG_SERIAL.print(uid[i], HEX);
            DEBUG_SERIAL.print(" ");
        }
        DEBUG_SERIAL.println("");
    }
};

#endif // RFIDMANAGER_H
