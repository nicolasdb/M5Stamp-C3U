#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include "config.h"

class StorageManager {
private:
    static const char* LOG_FILE;
    static const int MAX_LOG_ENTRIES = 100;
    bool isMounted;

    void trimLogFile() {
        String recentEntries = getRecentEntries(MAX_LOG_ENTRIES - 1);
        
        File file = SPIFFS.open(LOG_FILE, FILE_WRITE);
        if (!file) {
            DEBUG_SERIAL.println("Failed to open log file for trimming");
            return;
        }
        
        file.println("timestamp,tag_id,event_type,post_status");
        file.print(recentEntries);
        file.close();
        
        DEBUG_SERIAL.println("Log file trimmed successfully");
    }

    int countLogEntries() {
        File file = SPIFFS.open(LOG_FILE, FILE_READ);
        if (!file) {
            DEBUG_SERIAL.println("Failed to open log file for counting");
            return 0;
        }
        
        int count = -1;  // Start at -1 to skip header line
        while (file.available()) {
            file.readStringUntil('\n');
            count++;
        }
        
        file.close();
        return max(0, count);
    }

public:
    StorageManager() : isMounted(false) {}

    bool begin() {
        DEBUG_SERIAL.println("Mounting SPIFFS...");
        if (!SPIFFS.begin(true)) {
            DEBUG_SERIAL.println("SPIFFS mount failed. Formatting...");
            if (!SPIFFS.format()) {
                DEBUG_SERIAL.println("SPIFFS format failed");
                return false;
            }
            if (!SPIFFS.begin()) {
                DEBUG_SERIAL.println("SPIFFS mount failed after formatting");
                return false;
            }
        }
        DEBUG_SERIAL.println("SPIFFS mounted successfully");
        
        // Ensure log file exists with header
        if (!SPIFFS.exists(LOG_FILE)) {
            File file = SPIFFS.open(LOG_FILE, FILE_WRITE);
            if (file) {
                file.println("timestamp,tag_id,event_type,post_status");
                file.close();
            }
        }
        
        isMounted = true;
        return true;
    }

    bool writeLogEntry(const char* timestamp, const char* tagId, const char* eventType, bool postStatus) {
        if (!isMounted) return false;

        if (countLogEntries() >= MAX_LOG_ENTRIES) {
            trimLogFile();
        }
        
        File file = SPIFFS.open(LOG_FILE, FILE_APPEND);
        if (!file) {
            DEBUG_SERIAL.println("Failed to open log file for writing");
            return false;
        }
        
        // Format: timestamp,tag_id,event_type,post_status
        String entry = String(timestamp) + "," + 
                      String(tagId) + "," + 
                      String(eventType) + "," + 
                      String(postStatus ? "1" : "0") + "\n";
        
        if (file.print(entry)) {
            DEBUG_SERIAL.println("Log entry written successfully");
            file.close();
            return true;
        } else {
            DEBUG_SERIAL.println("Failed to write log entry");
            file.close();
            return false;
        }
    }

    String getRecentEntries(int count) {
        if (!isMounted) return "";

        String entries = "";
        File file = SPIFFS.open(LOG_FILE, FILE_READ);
        if (!file) {
            DEBUG_SERIAL.println("Failed to open log file for reading");
            return entries;
        }
        
        // Skip header line
        file.readStringUntil('\n');
        
        // Store all lines in an array
        String lines[MAX_LOG_ENTRIES];
        int lineCount = 0;
        
        while (file.available() && lineCount < MAX_LOG_ENTRIES) {
            String line = file.readStringUntil('\n');
            if (line.length() > 0) {
                lines[lineCount++] = line;
            }
        }
        
        file.close();
        
        // Get the most recent entries
        int start = max(0, lineCount - count);
        for (int i = start; i < lineCount; i++) {
            entries += lines[i];
            if (i < lineCount - 1) {
                entries += "\n";
            }
        }
        
        return entries;
    }

    bool mounted() const {
        return isMounted;
    }
};

const char* StorageManager::LOG_FILE = "/log.csv";

#endif // STORAGEMANAGER_H
