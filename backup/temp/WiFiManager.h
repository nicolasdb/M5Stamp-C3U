#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"
#include "credentials.h"

class WiFiManager {
private:
    bool isConnected;
    int currentNetworkIndex;
    unsigned long lastReconnectAttempt;

    bool connectToNetwork(const WiFiNetwork& network) {
        DEBUG_SERIAL.printf("Attempting to connect to %s\n", network.ssid);
        
        WiFi.begin(network.ssid, network.password);
        unsigned long startAttempt = millis();
        
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startAttempt > WIFI_TIMEOUT) {
                DEBUG_SERIAL.println("Connection timeout");
                return false;
            }
            delay(500);
            DEBUG_SERIAL.print(".");
        }
        
        DEBUG_SERIAL.printf("\nConnected to %s\n", network.ssid);
        DEBUG_SERIAL.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

public:
    WiFiManager() : isConnected(false), currentNetworkIndex(0), lastReconnectAttempt(0) {
        WiFi.mode(WIFI_STA);
    }

    bool begin() {
        DEBUG_SERIAL.println("Starting WiFi connection...");
        
        for (int i = 0; i < WIFI_NETWORKS_COUNT; i++) {
            if (connectToNetwork(WIFI_NETWORKS[i])) {
                isConnected = true;
                currentNetworkIndex = i;
                return true;
            }
            WiFi.disconnect();
            delay(1000);
        }
        
        DEBUG_SERIAL.println("Failed to connect to any network");
        return false;
    }

    bool checkConnection() {
        if (WiFi.status() != WL_CONNECTED) {
            isConnected = false;
            
            // Attempt reconnection every WIFI_TIMEOUT interval
            unsigned long currentTime = millis();
            if (currentTime - lastReconnectAttempt > WIFI_TIMEOUT) {
                lastReconnectAttempt = currentTime;
                DEBUG_SERIAL.println("Connection lost, attempting reconnection...");
                
                // Try the last successful network first
                if (connectToNetwork(WIFI_NETWORKS[currentNetworkIndex])) {
                    isConnected = true;
                    return true;
                }
                
                // If that fails, try all networks
                return begin();
            }
            return false;
        }
        isConnected = true;
        return true;
    }

    bool isWiFiConnected() const {
        return isConnected;
    }

    void disconnect() {
        WiFi.disconnect();
        isConnected = false;
    }
};

#endif // WIFI_MANAGER_H
