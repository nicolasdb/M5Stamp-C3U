#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "credentials.h"

class WiFiManager {
private:
    bool _isConnected;
    String _currentSSID;
    
    // Helper functions
    void updateLEDStatus(uint32_t color);
    bool connectToNetwork(const char* ssid, const char* password);

public:
    WiFiManager();
    
    // Core WiFi functions
    bool begin();  // Initialize WiFi and attempt connection
    bool connect();  // Try to connect to any available network
    void disconnect();
    bool isConnected() const { return _isConnected; }
    
    // Status and info
    String getCurrentSSID() const { return _currentSSID; }
    String getIPAddress() const;
    int getRSSI() const;
    
    // Connection management
    bool checkConnection();  // Verify current connection status
    bool reconnectIfNeeded();  // Attempt reconnection if disconnected
};

#endif // WIFI_MANAGER_H
