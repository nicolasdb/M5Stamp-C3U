#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <time.h>
#include "config.h"

class TimeManager {
private:
    struct tm timeinfo;
    bool isSynced;

public:
    TimeManager() : isSynced(false) {
        memset(&timeinfo, 0, sizeof(timeinfo));
    }

    bool begin() {
        DEBUG_SERIAL.println("Syncing time with NTP...");
        configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        
        int retries = 0;
        const int maxRetries = 5;
        
        while (!getLocalTime(&timeinfo) && retries < maxRetries) {
            DEBUG_SERIAL.println("Failed to obtain time, retrying...");
            delay(1000);
            retries++;
        }
        
        if (retries >= maxRetries) {
            DEBUG_SERIAL.println("Failed to sync time after maximum retries");
            isSynced = false;
            return false;
        }
        
        DEBUG_SERIAL.println("Time synchronized successfully");
        char timeStringBuff[50];
        strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
        DEBUG_SERIAL.printf("Current time: %s\n", timeStringBuff);
        
        isSynced = true;
        return true;
    }

    bool getCurrentTime(struct tm* time) {
        if (!getLocalTime(&timeinfo)) {
            DEBUG_SERIAL.println("Failed to obtain time");
            return false;
        }
        memcpy(time, &timeinfo, sizeof(struct tm));
        return true;
    }

    String getFormattedTime() {
        if (!getLocalTime(&timeinfo)) {
            return "";
        }
        char timeStringBuff[25];
        strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(timeStringBuff);
    }

    bool synced() const {
        return isSynced;
    }
};

#endif // TIMEMANAGER_H
