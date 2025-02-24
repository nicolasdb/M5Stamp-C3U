#include "wifi_manager.h"
#include <Adafruit_NeoPixel.h>

// External reference to NeoPixel object from main.cpp
extern Adafruit_NeoPixel pixels;

WiFiManager::WiFiManager() : _isConnected(false), _currentSSID("") {
}

void WiFiManager::updateLEDStatus(uint32_t color) {
    pixels.setPixelColor(0, color);
    pixels.show();
}

bool WiFiManager::begin() {
    DEBUG_SERIAL.println("\nInitializing WiFi...");
    WiFi.mode(WIFI_STA);
    return connect();
}

bool WiFiManager::connectToNetwork(const char* ssid, const char* password) {
    DEBUG_SERIAL.printf("Attempting to connect to %s\n", ssid);
    
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    
    // Connection attempt loop with LED feedback
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < WIFI_TIMEOUT) {
        updateLEDStatus(COLOR_WIFI_CONNECTING);
        delay(LED_FAST_BLINK_INTERVAL);
        updateLEDStatus(0);
        delay(LED_FAST_BLINK_INTERVAL);
        DEBUG_SERIAL.print(".");
    }
    DEBUG_SERIAL.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        _isConnected = true;
        _currentSSID = ssid;
        updateLEDStatus(COLOR_WIFI_CONNECTED);
        DEBUG_SERIAL.printf("Connected to %s\n", ssid);
        DEBUG_SERIAL.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    
    DEBUG_SERIAL.printf("Failed to connect to %s\n", ssid);
    return false;
}

bool WiFiManager::connect() {
    if (_isConnected) {
        return true;
    }
    
    DEBUG_SERIAL.println("Scanning available networks...");
    int numNetworks = WiFi.scanNetworks();
    
    if (numNetworks == 0) {
        DEBUG_SERIAL.println("No networks found!");
        updateLEDStatus(COLOR_ERROR);
        return false;
    }
    
    // Try to connect to each configured network in order
    for (int i = 0; i < WIFI_NETWORKS_COUNT; i++) {
        const char* targetSSID = WIFI_NETWORKS[i].ssid;
        const char* targetPass = WIFI_NETWORKS[i].password;
        
        // Check if this network is visible in scan results
        bool networkFound = false;
        for (int j = 0; j < numNetworks; j++) {
            if (WiFi.SSID(j) == targetSSID) {
                networkFound = true;
                DEBUG_SERIAL.printf("Found configured network: %s\n", targetSSID);
                break;
            }
        }
        
        if (networkFound && connectToNetwork(targetSSID, targetPass)) {
            return true;
        }
    }
    
    // If we get here, all connection attempts failed
    updateLEDStatus(COLOR_ERROR);
    return false;
}

void WiFiManager::disconnect() {
    if (_isConnected) {
        WiFi.disconnect();
        _isConnected = false;
        _currentSSID = "";
        updateLEDStatus(COLOR_ERROR);
        DEBUG_SERIAL.println("WiFi disconnected");
    }
}

String WiFiManager::getIPAddress() const {
    if (_isConnected) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

int WiFiManager::getRSSI() const {
    if (_isConnected) {
        return WiFi.RSSI();
    }
    return 0;
}

bool WiFiManager::checkConnection() {
    bool currentStatus = WiFi.status() == WL_CONNECTED;
    
    if (_isConnected != currentStatus) {
        _isConnected = currentStatus;
        if (!_isConnected) {
            _currentSSID = "";
            updateLEDStatus(COLOR_ERROR);
            DEBUG_SERIAL.println("WiFi connection lost!");
        }
    }
    
    return _isConnected;
}

bool WiFiManager::reconnectIfNeeded() {
    if (!checkConnection()) {
        DEBUG_SERIAL.println("Attempting to reconnect...");
        return connect();
    }
    return true;
}
