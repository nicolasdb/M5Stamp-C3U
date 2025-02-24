#include "webhook_manager.h"

WebhookManager::WebhookManager() {
    webhookUrl = WEBHOOK_URL;
}

bool WebhookManager::begin() {
    return true;
}

bool WebhookManager::sendPollResult(bool tagPresent, String currentTagId, String lastTagId,
                                  String tagType, String wifiStatus, String timeStatus,
                                  String timestamp) {
    // Create JSON document
    StaticJsonDocument<512> doc;
    JsonObject rfidPollResult = doc.createNestedObject("rfid_poll_result");
    
    rfidPollResult["timestamp"] = timestamp;
    rfidPollResult["event_type"] = tagPresent ? "tag_insert" : "tag_removed";
    rfidPollResult["tag_present"] = tagPresent;
    rfidPollResult["tag_id"] = tagPresent ? currentTagId : lastTagId;
    
    if (tagPresent) {
        rfidPollResult["tag_type"] = tagType;
        rfidPollResult["wifi_status"] = wifiStatus;
        rfidPollResult["time_status"] = timeStatus;
    }

    // Serialize JSON to string
    String jsonString;
    serializeJson(doc, jsonString);

    // Send HTTP POST request
    http.begin(webhookUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);
    bool success = (httpResponseCode > 0 && httpResponseCode < 300);
    
    http.end();
    return success;
}
