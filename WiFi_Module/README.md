# WiFi Module - Wireless Data Transmission

This folder contains the ESP32 code for wireless transmission of wind data via MQTT protocol.

## Overview

The WiFi module receives JSON wind data from the anemometer via serial communication and publishes it to an MQTT broker for remote data collection and analysis.

## Hardware Requirements

- **ESP32 Development Board** (one of the following):
  - ESP32-S3-DevKitC-1 (recommended)
  - ESP32-DevKit-V4
- **Serial connection** to Arduino anemometer (3.3V/5V compatible)
- **USB cable** for programming and power
- **WiFi network** access

## Pin Connections

### Serial Communication with Anemometer
- **RX Pin** - Connect to Arduino TX (Serial output from anemometer)
- **GND** - Common ground between ESP32 and Arduino
- **Power** - Ensure compatible voltage levels (3.3V/5V)

**Note:** ESP32 serial pins are typically GPIO1 (TX) and GPIO3 (RX) for Serial0, but this varies by board model.

## Software Setup

### 1. Install PlatformIO
Install PlatformIO extension in VS Code or use PlatformIO Core CLI.

### 2. Create Secrets File
Create `include/arduino_secrets.h` with your network credentials:

```cpp
#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

// WiFi Credentials
#define SECRET_SSID "your_wifi_network_name"
#define SECRET_PASS "your_wifi_password"

// MQTT Credentials  
#define SECRET_MQTTUSER "your_mqtt_username"
#define SECRET_MQTTPASS "your_mqtt_password"

#endif
```

**Security Note:** This file is in `.gitignore` to prevent credential exposure.

### 3. Select Board Environment
The `platformio.ini` defines two environments:

```ini
[env:esp32-s3-devkitc-1]    # For ESP32-S3 boards
[env:esp32-devkitC_v4]      # For ESP32 DevKit V4 boards
```

### 4. Upload Code
```bash
# For ESP32-S3
pio run --target upload --environment esp32-s3-devkitc-1

# For ESP32 DevKit V4  
pio run --target upload --environment esp32-devkitC_v4
```

## MQTT Configuration

### Broker Settings
The code is pre-configured for the CETools MQTT broker:
- **Host:** `mqtt.cetools.org`
- **Port:** `1884`
- **Topic:** `student/ultrasonic_anemometer`

### Topic Structure
Data is published to: `student/ultrasonic_anemometer`

### Authentication
Uses username/password authentication as defined in `arduino_secrets.h`.

## Code Structure

### Main Components

#### `main.cpp`
- **WiFi Management:** Automatic connection and reconnection
- **MQTT Client:** Handles broker connection and publishing
- **Serial Communication:** Receives data from anemometer at 9600 baud
- **Data Forwarding:** Publishes received JSON data to MQTT

### Key Functions

#### `startWifi()`
- Initializes WiFi connection
- Displays connection status and IP address
- Blocks until connected

#### `checkWifi()`
- Monitors WiFi connection status
- Automatically reconnects if disconnected
- Called every 10 seconds (INTERVAL)

#### `checkMQTT()`
- Maintains MQTT broker connection
- Handles authentication and reconnection
- Called continuously in main loop

#### `sendMQTT(char *msg)`
- Publishes wind data to MQTT topic
- Provides confirmation feedback via Serial
- Handles publish failures gracefully

### Connection Management
The system implements robust connection handling:
- **WiFi Auto-Reconnect:** Checks connection every 10 seconds
- **MQTT Persistence:** Maintains broker connection continuously
- **Error Recovery:** Automatic retry with delays
- **Status Reporting:** Serial output for debugging

## Data Flow

```
[Arduino] --Serial 9600--> [ESP32] --WiFi--> [MQTT Broker] ---> [Data Collector]
```

1. **Serial Input:** ESP32 receives JSON strings from Arduino
2. **Format Validation:** Accepts complete lines ending with `\n`
3. **MQTT Publishing:** Forwards data to configured topic
4. **Confirmation:** Serial feedback confirms successful transmission

## Expected Data Format

The system expects JSON wind data from the anemometer:
```json
{"speed":5.50,"direction":126}
```

Invalid formats are discarded with error messages.

## Monitoring and Debugging

### Serial Monitor Output
Connect to ESP32 at 9600 baud to monitor:
```
ESP32 MQTT
Connecting to WiFi..
Connected: 192.168.1.100
Attempting MQTT connection...connected
SENDING MSG...
Topic: student/ultrasonic_anemometer  
Message: {"speed":5.50,"direction":126}
Message published
```

### Connection Status Messages
- **WiFi:** IP address displayed on successful connection
- **MQTT:** Connection confirmation with client ID
- **Publish:** Topic and message content logged
- **Errors:** Failed connections and retry attempts

## Troubleshooting

### WiFi Connection Issues
1. **Check Credentials:**
   - Verify SSID and password in `arduino_secrets.h`
   - Ensure network is 2.4GHz (ESP32 limitation)

2. **Network Compatibility:**
   - Test with mobile hotspot
   - Check for enterprise/captive portal networks

3. **Signal Strength:**
   - Move closer to WiFi router
   - Check antenna connection on ESP32

### MQTT Connection Problems
1. **Broker Access:**
   - Verify `mqtt.cetools.org` is accessible
   - Test with MQTT client tools (mosquitto_pub/sub)

2. **Credentials:**
   - Confirm MQTT username/password
   - Check account permissions

3. **Firewall Issues:**
   - Ensure port 1884 is not blocked
   - Test from different network

### Serial Communication Issues
1. **Arduino Connection:**
   - Check serial wiring between Arduino and ESP32
   - Verify ground connection
   - Confirm voltage compatibility (3.3V/5V)

2. **Baud Rate:**
   - Both devices must use 9600 baud
   - Check Arduino serial output first

3. **Data Format:**
   - ESP32 expects complete lines (ending with `\n`)
   - Arduino should use `Serial.println()`

### No Data Publishing
1. **Check Serial Input:**
   - Monitor ESP32 serial for incoming messages
   - Verify Arduino is sending data

2. **MQTT Connection:**
   - Ensure MQTT client shows "connected" status
   - Test topic subscription with external client

3. **Message Format:**
   - ESP32 forwards received messages directly
   - Any string from Arduino will be published

## Performance Specifications

- **Serial Baud Rate:** 9600 bps
- **WiFi Standard:** 802.11 b/g/n (2.4GHz)
- **MQTT QoS:** 0 (fire-and-forget)
- **Reconnection Interval:** 10 seconds (WiFi), 5 seconds (MQTT)
- **Message Rate:** Unlimited (forwards all received data)

## Power Considerations

- **USB Power:** Suitable for development and testing
- **External Power:** 3.3V-5V DC input for permanent installation
- **Power Consumption:** ~160mA active, ~10μA deep sleep (if implemented)

## Libraries Used

- **WiFi:** ESP32 built-in WiFi library
- **PubSubClient:** MQTT client implementation (v2.8+)
- **ArduinoJson:** JSON formatting library (v7.4.2+)

These dependencies are automatically managed by PlatformIO.