# NFC Time Tracking Device with M5Stamp C3U

## Overview

This project implements an NFC-based time tracking device using the M5Stamp C3U microcontroller and PN532 RFID module. The device tracks work sessions by detecting NFC tags and sends event data with health metrics to n8n via webhooks.

## Features

- Real-time NFC tag detection
- WiFi connectivity
- LED status indicators
- Webhook integration with n8n
- Automatic health metrics in every event

## Hardware Requirements

- M5Stamp C3U
- PN532 NFC Module
- SK6812 LED (built-in)

## Pin Configuration

- I2C SDA: Pin 4
- I2C SCL: Pin 3
- LED: Pin 2

## LED Status Indicators

- Blue: Waiting for tag
- Green: Tag present
- Red: Error state (WiFi disconnection, NFC module not found)

## Configuration

### credentials.h

Create `src/credentials.h` with the following structure:

```cpp
// WiFi Configuration
#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"

// n8n Webhook
#define WEBHOOK_URL "your_webhook_url"

// Optional: Device identification (leave empty to use chip ID)
#define DEVICE_NAME ""
```

## Webhook Payload Structure

The device sends a unified JSON payload for all events, including both tag status and health metrics:

```json
{
  "event_type": "tag_event",  // or "health_check" when no tag interaction
  "device_id": "NFC_ABCDEF",
  "timestamp": "1234567890",
  "tag_status": {
    "tag_id": "04E4D4BA",    // empty for health checks
    "action": "placed"        // "placed", "removed", or empty for health checks
  },
  "health_metrics": {
    "wifi_strength": -65,
    "uptime": 3600,
    "free_memory": 204800
  }
}
```

## Dependencies

- FastLED
- Adafruit PN532
- ArduinoJson

## Workflow Integration

The device sends webhook events that can be processed by n8n to:

- Track time entries when tags are placed/removed
- Monitor device health (included in every payload)
- Log tag interactions with comprehensive device status

## Development

1. Install PlatformIO
2. Clone the repository
3. Configure `credentials.h`
4. Upload to M5Stamp C3U

## Troubleshooting

- Ensure WiFi credentials are correct
- Check webhook URL
- Verify NFC module connection
- Monitor serial output for detailed logs

## License

[Insert your license information]

## Contributing

Contributions are welcome! Please submit pull requests or open issues.
