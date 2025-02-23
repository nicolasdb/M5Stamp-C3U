#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include "credentials.h"
#include "config.h"

// LED setup
CRGB led[1];

// Global variables
bool buttonPressed = false;
time_t lastButtonPress = 0;
struct tm timeinfo;

// Function declarations
void setupLED();
void setupButton();
bool connectWiFi();
void syncTime();
void setLEDColor(uint32_t color);
void handleButtonPress();
void sendPayload();

void setup() {
    // Initialize serial communication
    DEBUG_SERIAL.begin(SERIAL_BAUD);
    DEBUG_SERIAL.println("\nStarting M5Stamp-C3U Time Tracker V3...");

    // Setup hardware
    setupLED();
    setupButton();

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
    DEBUG_SERIAL.println("Button pressed, sending payload...");
    sendPayload();
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
    StaticJsonDocument<200> doc;
    doc["device_name"] = DEVICE_NAME;
    doc["timestamp"] = timeStringBuff;
    doc["tag_id"] = "DUMMY_TAG_001";  // Placeholder
    doc["event_type"] = "button_press";

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
