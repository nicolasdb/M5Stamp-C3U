# Time Tracker Project Progress V3

## Core Design Principles
1. FSM (Finite State Machine) as main mechanic for tag states and behaviors
2. Serial print monitoring for feedback and debug
3. Structured and documented code for clear organization
4. Modular functions (input-process-output model)
5. Step-by-step integration with validation checkpoints

## Implementation Phases

### Phase 1: Basic Infrastructure
- [ ] Multi SSID connection from credentials.h array
- [ ] NTP time sync for internal clock calibration
- [ ] Button trigger (GPIO 9) for JSON payload POST
- [ ] Basic webhook defined in credentials.h
- [ ] Validation: Successful POST with:
  * Device name
  * Internal time
  * Dummy tagID
  * Dummy event type

### Phase 2: SPIFFS Integration
- [ ] Create and manage log.csv on flash partition
- [ ] Button still triggers payload
- [ ] Add log.csv content to JSON payload
- [ ] Validation: Verify read/write operations and payload inclusion

### Phase 3: LED Status & RFID Integration
- [ ] Define LED status indicators
- [ ] Implement RFID module communication
- [ ] Read tag ID (1 sec interval)
- [ ] Evaluate tag states:
  * Insert
  * StillHere
  * Removed
- [ ] Send states as JSON payload

### Phase 4: NFC Behavior Optimization
- [ ] Implement selective webhook sending
- [ ] Manage log.csv (max 100 entries)
- [ ] Tag state logic:
  * Empty reader = break
  * New tag = log + payload + break
  * Existing tag = break
  * Tag removal = log + payload
- [ ] Add post status tracking (boolean field)
- [ ] Queue failed payloads
- [ ] Retry on WiFi reconnection

## Current Status
Starting implementation of Phase 1: Basic Infrastructure
