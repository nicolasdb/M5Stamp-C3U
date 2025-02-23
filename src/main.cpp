#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "credentials.h"
#include "config.h"

// LED setup
CRGB led[1];

// Global variables
bool buttonPressed = false;
time_t lastButtonPress = 0;
struct tm timeinfo;

// Constants
const char* LOG_FILE = "/log.csv";
const int MAX_LOG_ENTRIES = 100;
const int ENTRIES_TO_INCLUDE = 3;

// Function declarations
void setupLED();
void setupButton();
void setupSPIFFS();
bool connectWiFi();
void syncTime();
void setLEDColor(uint32_t color);
void handleButtonPress();
void sendPayload();
bool writeLogEntry(const char* timestamp, const char* tagId, const char* eventType, bool postStatus);
String getRecentEntries(int count);
int countLogEntries();
void trimLogFile();

void setup() {
    // Initialize serial communication
    DEBUG_SERIAL.begin(SERIAL_BAUD);
    DEBUG_SERIAL.println("\nStarting M5Stamp-C3U Time Tracker V3...");

    // Setup hardware and systems
    setupLED();
    setupButton();
    setupSPIFFS();

    // Connect to WiFi
    if (!connectWiFi()) {
        DEBUG_SERIAL.println("Failed to connect to WiFi. Restarting...");
        ESP.restart();
    }

    // Sync time
    syncTime();
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {  // Button is active LOW
        if (!buttonPressed && (millis() - lastButtonPress > BUTTON_DEBOUNCE_TIME)) {
            buttonPressed = true;
            lastButtonPress = millis();
            handleButtonPress();
        }
    } else {
        buttonPressed = false;
    }
}

void setupLED() {
    FastLED.addLeds<WS2812, BUILTIN_LED_PIN, GRB>(led, 1);
    FastLED.setBrightness(LED_BRIGHTNESS);
    setLEDColor(COLOR_WIFI_CONNECTING);
}

void setupButton() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void setupSPIFFS() {
    DEBUG_SERIAL.println("Mounting SPIFFS...");
    if (!SPIFFS.begin(true)) {
        DEBUG_SERIAL.println("SPIFFS mount failed. Formatting...");
        if (!SPIFFS.format()) {
            DEBUG_SERIAL.println("SPIFFS format failed");
            setLEDColor(COLOR_ERROR);
            return;
        }
        if (!SPIFFS.begin()) {
            DEBUG_SERIAL.println("SPIFFS mount failed after formatting");
            setLEDColor(COLOR_ERROR);
            return;
        }
    }
    DEBUG_SERIAL.println("SPIFFS mounted successfully");
}

bool connectWiFi() {
    DEBUG_SERIAL.println("Attempting to connect to WiFi...");
    
    for (int i = 0; i < WIFI_NETWORKS_COUNT; i++) {
        DEBUG_SERIAL.printf("Trying to connect to %s\n", WIFI_NETWORKS[i].ssid);
        
        WiFi.begin(WIFI_NETWORKS[i].ssid, WIFI_NETWORKS[i].password);
        
        unsigned long startAttemptTime = millis();
        
        while (WiFi.status() != WL_CONNECTED && 
               millis() - startAttemptTime < WIFI_TIMEOUT) {
            delay(100);
            DEBUG_SERIAL.print(".");
        }
        DEBUG_SERIAL.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            DEBUG_SERIAL.printf("Connected to %s\n", WIFI_NETWORKS[i].ssid);
            DEBUG_SERIAL.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
            setLEDColor(COLOR_WIFI_CONNECTED);
            return true;
        }
        
        DEBUG_SERIAL.printf("Failed to connect to %s\n", WIFI_NETWORKS[i].ssid);
        WiFi.disconnect();
    }
    
    setLEDColor(COLOR_ERROR);
    return false;
}

void syncTime() {
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
        setLEDColor(COLOR_ERROR);
        return;
    }
    
    DEBUG_SERIAL.println("Time synchronized successfully");
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    DEBUG_SERIAL.printf("Current time: %s\n", timeStringBuff);
}

void setLEDColor(uint32_t color) {
    led[0] = color;
    FastLED.show();
}

void handleButtonPress() {
    DEBUG_SERIAL.println("Button pressed, logging entry and sending payload...");
    
    // Get current time
    if (!getLocalTime(&timeinfo)) {
        DEBUG_SERIAL.println("Failed to obtain time");
        return;
    }

    char timeStringBuff[25];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    // Write log entry
    if (!writeLogEntry(timeStringBuff, "DUMMY_TAG_001", "button_press", true)) {
        DEBUG_SERIAL.println("Failed to write log entry");
        return;
    }
    
    sendPayload();
}

bool writeLogEntry(const char* timestamp, const char* tagId, const char* eventType, bool postStatus) {
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

void sendPayload() {
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_SERIAL.println("WiFi not connected. Attempting to reconnect...");
        if (!connectWiFi()) {
            DEBUG_SERIAL.println("Failed to reconnect to WiFi");
            return;
        }
    }

    // Get current time
    if (!getLocalTime(&timeinfo)) {
        DEBUG_SERIAL.println("Failed to obtain time");
        return;
    }

    char timeStringBuff[25];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Create JSON payload
    DynamicJsonDocument doc(1024);  // Increased size for log entries
    doc["device_name"] = DEVICE_NAME;
    doc["timestamp"] = timeStringBuff;
    doc["tag_id"] = "DUMMY_TAG_001";
    doc["event_type"] = "button_press";

    // Add recent log entries
    String recentLogs = getRecentEntries(ENTRIES_TO_INCLUDE);
    if (recentLogs.length() > 0) {
        JsonArray logEntries = doc.createNestedArray("log_entries");
        
        int start = 0;
        while (start < recentLogs.length()) {
            int end = recentLogs.indexOf('\n', start);
            if (end == -1) end = recentLogs.length();
            
            String line = recentLogs.substring(start, end);
            if (line.length() > 0) {
                // Parse CSV line
                int comma1 = line.indexOf(',');
                int comma2 = line.indexOf(',', comma1 + 1);
                int comma3 = line.indexOf(',', comma2 + 1);
                
                JsonObject entry = logEntries.createNestedObject();
                entry["timestamp"] = line.substring(0, comma1);
                entry["tag_id"] = line.substring(comma1 + 1, comma2);
                entry["event_type"] = line.substring(comma2 + 1, comma3);
                entry["post_status"] = line.substring(comma3 + 1).equals("1");
            }
            
            start = end + 1;
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);

    // Send HTTP POST request
    HTTPClient http;
    http.begin(WEBHOOK_URL);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
        DEBUG_SERIAL.printf("HTTP Response code: %d\n", httpResponseCode);
        String response = http.getString();
        DEBUG_SERIAL.println(response);
    } else {
        DEBUG_SERIAL.printf("Error code: %d\n", httpResponseCode);
    }
    
    http.end();
}
