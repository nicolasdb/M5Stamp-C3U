#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

class LEDManager {
private:
    CRGB led[1];
    bool isInitialized;
    uint32_t currentColor;

public:
    LEDManager() : isInitialized(false), currentColor(0) {}

    bool begin() {
        FastLED.addLeds<WS2812, BUILTIN_LED_PIN, GRB>(led, 1);
        FastLED.setBrightness(LED_BRIGHTNESS);
        setColor(COLOR_WIFI_CONNECTING);  // Blue while starting
        isInitialized = true;
        return true;
    }

    void showState(bool error = false, bool wifi = false, bool tag = false) {
        if (!isInitialized) return;

        uint32_t newColor;
        if (error) {
            newColor = COLOR_ERROR;  // Red for errors
        } else if (tag) {
            newColor = COLOR_TAG_PRESENT;  // Green for tag present
        } else {
            newColor = COLOR_WIFI_CONNECTING;  // Blue for ready/waiting
        }

        // Only update if color changed
        if (newColor != currentColor) {
            currentColor = newColor;
            setColor(currentColor);
            DEBUG_SERIAL.printf("LED State Change: 0x%06X\n", currentColor);
        }
    }

    void showError() {
        if (!isInitialized) return;
        currentColor = COLOR_ERROR;
        setColor(COLOR_ERROR);
        DEBUG_SERIAL.println("LED Error State");
    }

private:
    void setColor(uint32_t color) {
        if (!isInitialized) return;
        led[0] = color;
        FastLED.show();
    }
};

#endif // LEDMANAGER_H
