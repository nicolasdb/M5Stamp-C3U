#ifndef WEBHOOKMANAGER_H
#define WEBHOOKMANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "config.h"
#include "credentials.h"

class WebhookManager {
private:
    bool lastSendSuccess;
    static const int ENTRIES_TO_INCLUDE = 3;
    static constexpr const char* QUEUE_FILE = "/webhook_queue.json";
    static const int MAX_QUEUE_SIZE = 50;  // Maximum number of queued payloads

    struct QueuedPayload {
        String tagId;
        String eventType;
        String timestamp;
        int retryCount;
    };

public:
    WebhookManager() : lastSendSuccess(false) {
        // Ensure queue file exists
        if (!SPIFFS.exists(QUEUE_FILE)) {
            File file = SPIFFS.open(QUEUE_FILE, "w");
            if (file) {
                DynamicJsonDocument doc(1024);
                doc["queue"] = JsonArray();
                serializeJson(doc, file);
                file.close();
            }
        }
    }

    bool sendPayload(const String& tagId, const char* eventType, const String& timestamp, const String& recentLogs, bool isRetry = false) {
        HTTPClient http;
        http.begin(WEBHOOK_URL);
        http.addHeader("Content-Type", "application/json");

        // Create JSON payload
        DynamicJsonDocument doc(1024);  // Increased size for log entries
        doc["device_name"] = DEVICE_NAME;
        doc["timestamp"] = timestamp;
        doc["tag_id"] = tagId;
        doc["event_type"] = eventType;

        // Add recent log entries if available
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
        int httpResponseCode = http.POST(jsonString);
        
        if (httpResponseCode > 0) {
            DEBUG_SERIAL.printf("HTTP Response code: %d\n", httpResponseCode);
            String response = http.getString();
            DEBUG_SERIAL.println(response);
            lastSendSuccess = (httpResponseCode == 200);
        } else {
            DEBUG_SERIAL.printf("Error code: %d\n", httpResponseCode);
            lastSendSuccess = false;
        }
        
        http.end();

        // If send failed and this isn't already a retry attempt, queue the payload
        if (!lastSendSuccess && !isRetry) {
            queuePayload(tagId, eventType, timestamp);
        }
        
        return lastSendSuccess;
    }

    bool isLastSendSuccessful() const {
        return lastSendSuccess;
    }

    int getEntriesToInclude() const {
        return ENTRIES_TO_INCLUDE;
    }

    bool hasQueuedPayloads() {
        File file = SPIFFS.open(QUEUE_FILE, "r");
        if (!file) return false;

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) return false;
        return doc["queue"].size() > 0;
    }

    void processQueue() {
        if (!hasQueuedPayloads()) return;

        File file = SPIFFS.open(QUEUE_FILE, "r");
        if (!file) return;

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) return;

        JsonArray queue = doc["queue"];
        bool queueChanged = false;

        // Process each queued payload
        for (int i = queue.size() - 1; i >= 0; i--) {
            JsonObject payload = queue[i];
            String tagId = payload["tagId"].as<String>();
            String eventType = payload["eventType"].as<String>();
            String timestamp = payload["timestamp"].as<String>();
            int retryCount = payload["retryCount"] | 0;

            // Try to send the payload
            if (sendPayload(tagId, eventType.c_str(), timestamp, "", true)) {
                // Success - remove from queue
                queue.remove(i);
                queueChanged = true;
            } else if (retryCount < 3) {  // Max 3 retry attempts
                // Update retry count
                payload["retryCount"] = retryCount + 1;
                queueChanged = true;
            }
        }

        // Save updated queue if changed
        if (queueChanged) {
            File writeFile = SPIFFS.open(QUEUE_FILE, "w");
            if (writeFile) {
                serializeJson(doc, writeFile);
                writeFile.close();
            }
        }
    }

private:
    void queuePayload(const String& tagId, const char* eventType, const String& timestamp) {
        File file = SPIFFS.open(QUEUE_FILE, "r");
        if (!file) return;

        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) return;

        JsonArray queue = doc["queue"];
        
        // Check queue size
        if (queue.size() >= MAX_QUEUE_SIZE) {
            // Remove oldest entry if queue is full
            queue.remove(0);
        }

        // Add new payload to queue
        JsonObject newPayload = queue.createNestedObject();
        newPayload["tagId"] = tagId;
        newPayload["eventType"] = eventType;
        newPayload["timestamp"] = timestamp;
        newPayload["retryCount"] = 0;

        // Save updated queue
        File writeFile = SPIFFS.open(QUEUE_FILE, "w");
        if (writeFile) {
            serializeJson(doc, writeFile);
            writeFile.close();
        }
    }
};

#endif // WEBHOOKMANAGER_H
