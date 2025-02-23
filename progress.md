# Time Tracker Project Progress V3

## Core Design Principles

1. FSM (Finite State Machine) as main mechanic for tag states and behaviors
2. Serial print monitoring for feedback and debug
3. Structured and documented code for clear organization
4. Modular functions (input-process-output model)
5. Step-by-step integration with validation checkpoints

## Implementation Phases

### Phase 1: Basic Infrastructure ✅

- [x] Multi SSID connection from credentials.h array
- [x] NTP time sync for internal clock calibration
- [x] Button trigger (GPIO 9) for JSON payload POST
- [x] Basic webhook defined in credentials.h
- [x] Validation: Successful POST with:
  - Device name
  - Internal time
  - Dummy tagID
  - Event type

Validation Results:

- WiFi: Connected successfully
- NTP Time Sync: Working correctly
- Button Trigger: Functioning as expected
- Payload POST: Successfully sending to webhook
- n8n Integration: Successfully mapping JSON fields to Supabase table

### Phase 2: SPIFFS Integration

- [x] Initialize SPIFFS and data structure:
  - Create /data folder
  - Setup log.csv with headers
  - Implement file management functions

- [x] Log Entry Implementation:
  - CSV Structure: timestamp,tag_id,event_type,post_status
  - Maximum 100 entries management
  - Read/Write operations validation

- [x] Enhanced Payload Structure:
  - Keep current webhook functionality
  - Add log_entries array to payload
  - Include last 3 entries for state tracking

Validation Checkpoints:

- [x] SPIFFS mount successful
- [x] log.csv creation/access working
- [x] Entry writing successful
- [x] Log content correctly included in webhook payload

Validation Results:

- SPIFFS: Successfully mounted and operational
- Log Management: Successfully creating and accessing log.csv
- Entry System: Writing entries on button press
- Payload Enhancement: Successfully including last 3 log entries
- All systems integrated and working as expected

Implementation Notes:

- SPIFFS filesystem configured in platformio.ini
- Log management functions implemented (write, read, trim)
- JSON payload enhanced to include recent log entries

### Phase 3: LED Status & RFID Integration

- [ ] LED Status Implementation:
  - Blue (WiFi Status):
    - Slow Blink (1s): Attempting connection
    - Solid Blue: Connected and stable
    - Fast Blink (0.2s): Reconnecting
  - Green (Tag Status):
    - Off: No tag (WiFi connected)
    - Solid Green: Tag detected
    - Slow Blink (1s): Payload queued
    - Fast Blink (0.2s): Processing state change
  - Red (Error States):
    - Hardware Errors:
      - 1 Blink: NFC module error
      - 2 Blinks: SPIFFS error
      - 3 Blinks: NTP error
    - Communication Errors:
      - Solid Red: Critical error
      - Slow Blink (1s): Webhook failed
      - Fast Blink (0.2s): Multiple retries failed

- [ ] RFID Integration:
  - Initialize PN532 module
  - Implement 1-second read interval
  - Handle tag state changes:
    - Insert (new tag detected)
    - StillHere (same tag present)
    - Removed (tag gone)
  - Update LED based on states
  - Send state changes via webhook

Validation Checkpoints:

- [ ] LED patterns working for all states
- [ ] PN532 communication established
- [ ] Tag reading at 1-second intervals
- [ ] State changes correctly detected
- [ ] Webhook payload includes tag states

### Phase 4: NFC Behavior Optimization
- [ ] Implement selective webhook sending
- [ ] Manage log.csv (max 100 entries)
- [ ] Tag state logic:
  - Empty reader = break
  - New tag = log + payload + break
  - Existing tag = break
  - Tag removal = log + payload
- [ ] Add post status tracking (boolean field)
- [ ] Queue failed payloads
- [ ] Retry on WiFi reconnection

## Current Status
Phase 1 completed successfully. Starting Phase 2 implementation with SPIFFS integration and enhanced logging functionality.
