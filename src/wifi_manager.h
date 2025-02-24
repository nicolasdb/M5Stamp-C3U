#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "credentials.h"

class WiFiManager {
private:
    bool _isConnected;
    bool _timeIsSynced;
    time_t _lastSyncTime;
    String _currentSSID;
    
    // Constants
    static const int NTP_SYNC_INTERVAL = 3600;  // Resync every hour
    static const int NTP_SYNC_TIMEOUT = 5000;   // 5 seconds timeout for sync
    
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
    
    // Time synchronization
    bool syncTime();  // Force NTP sync
    bool isTimeSynced() const { return _timeIsSynced; }
    time_t getLastSyncTime() const { return _lastSyncTime; }
    
    // Time getters
    String getFormattedTime() const;  // Returns current time as string
    time_t getCurrentTime() const;    // Returns current time as unix timestamp
};

#endif // WIFI_MANAGER_H
