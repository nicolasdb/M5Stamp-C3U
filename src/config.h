#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
#define BUILTIN_LED_PIN 2
#define BUTTON_PIN 9

// Time Configuration
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600  // UTC+1 Brussels
#define DAYLIGHT_OFFSET_SEC 3600

// Device Configuration
#define DEVICE_NAME "M5STAMP_C3U"  // Default name if not overridden
#define SERIAL_BAUD 115200

// LED Colors (RGB format)
#define COLOR_WIFI_CONNECTING 0x0000FF  // Blue
#define COLOR_WIFI_CONNECTED 0x00FF00   // Green
#define COLOR_ERROR 0xFF0000            // Red
#define LED_BRIGHTNESS 30               // 0-255 (default: 30)

// Timing Configuration
#define WIFI_TIMEOUT 10000        // ms to wait for WiFi connection
#define BUTTON_DEBOUNCE_TIME 200  // ms
#define HTTP_TIMEOUT 5000         // ms

// Debug Configuration
#define DEBUG_SERIAL Serial       // Use USB CDC serial for debug output

#endif // CONFIG_H
