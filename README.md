# Ultrasonic Anemometer Project

A complete 2D ultrasonic wind speed and direction measurement system with wireless data transmission and statistical analysis capabilities.

## Project Overview

This project implements a sonic anemometer that measures wind speed and direction using ultrasonic transducers. The system consists of three main components:

1. **Anemometer Device** - Arduino-based sensor that calculates wind measurements.
2. **WiFi Module** - ESP32-based wireless transmission system.
3. **Data Collection & Analysis** - Python scripts for data gathering and statistical analysis.
4. **CAD** - CAD Files for initial designs for 3D anemometer, and completed 2D anemometer. Along with the designs and laser-cut drawings for the model.

## System Architecture

```
[Ultrasonic Signal Board] → [Arduino Uno] → [Serial] → [ESP32] → [MQTT] → [Python Data Collector]
                                    ↓
                            [7-Segment Display]
```

## Hardware Requirements

### Anemometer Components
- Arduino Uno
- 3x Ultrasonic RX transducers (North, East, West)
- 1x Ultrasonic TX transducer
- 4-digit 7-segment display (common cathode)
- Various electronic components
- PCB

### WiFi Module Components
- ESP32 development board (ESP32-S3-DevKitC-1)
- Serial connection to anemometer Arduino

### Additional Components
- 9V Wall Wart with power pigtails
- Enclosure/mounting hardware (see CAD folder)

## Software Requirements

- **PlatformIO** (recommended) or Arduino IDE
- **Python 3.7+** with packages:
  - `paho-mqtt`
  - `pandas`
  - `numpy`
  - `statsmodels`
  - `scipy`

## Quick Start Guide

### 1. Hardware Setup
**_NOTE:_**  Please see [Instructable](https://www.instructables.com/Sonic-Anemometer/) for help in assembly. 
1. Build the anemometer circuit following the pin configurations in `anemometer/src/main.cpp`
2. Connect ultrasonic transducers
3. Wire the 7-segment display to digital pins 2-13
4. Connect Arduino to ESP32 via serial (9600 baud)

### 2. Software Installation
1. Install PlatformIO in VS Code
2. Clone this repository
3. Install Python dependencies:
   ```bash
   pip install paho-mqtt pandas numpy statsmodels scipy
   ```

### 3. Configuration
1. Create `arduino_secrets.h` in `WiFi_Module/include/` with your credentials:
   ```cpp
   #define SECRET_SSID "your_wifi_ssid"
   #define SECRET_PASS "your_wifi_password"
   #define SECRET_MQTTUSER "your_mqtt_username"
   #define SECRET_MQTTPASS "your_mqtt_password"
   ```

### 4. Upload Code
1. Upload `anemometer` code to Arduino Micro/Uno
2. Upload `WiFi_Module` code to ESP32

### 5. Data Collection
1. Run the MQTT collector:
   ```bash
   cd data_collector
   python mqtt_collector.py
   ```
2. Follow prompts to collect 200 readings
3. Use `merger.py` to combine multiple CSV files
4. Run `analysis.py` for statistical analysis

## Calibration

The anemometer requires calibration in three conditions:
1. **Still air** (calm conditions)
2. **Known north wind** (9 m/s reference)
3. **Known east wind** (9 m/s reference)

Update the calibration values in `anemometer/src/main.cpp`:
```cpp
const Wind_t calm =   {547, 581}; // Still air readings
const Wind_t north =  {850, 226}; // North wind readings  
const Wind_t east =   {720, 712}; // East wind readings
```

## Data Format

The system outputs JSON-formatted wind data:
```json
{"speed": 5.50, "direction": 126}
```

Where:
- `speed`: Wind speed in m/s
- `direction`: Wind direction in degrees (0-360°, meteorological convention)

## Folder Structure

```
├── anemometer/          # Arduino code for wind calculations
│   ├── src/main.cpp     # Main anemometer logic
│   ├── lib/sevSeg/      # 7-segment display library
│   └── platformio.ini   # Build configuration
├── WiFi_Module/         # ESP32 WiFi transmission code
│   ├── src/main.cpp     # MQTT publishing logic
│   └── platformio.ini   # ESP32 build configuration
├── data_collector/      # Python data collection tools
│   ├── mqtt_collector.py   # MQTT data collection script
│   ├── merger.py           # CSV file merger utility
│   └── analysis.py         # Statistical analysis tools
└── CAD/                 # 3D models and mechanical designs
    ├── Device             # STL files to print 2D Anemometer
    └── Exhibit            # Mix of STL and DXF files to make scaled
```

## Detailed Documentation

Each folder contains its own README with specific setup instructions:
- [Anemometer README](anemometer/README.md) - Arduino setup and calibration
- [WiFi Module README](WiFi_Module/README.md) - ESP32 configuration and MQTT setup
- [Data Collector README](data_collector/README.md) - Python tools and analysis

## Troubleshooting

### Common Issues

**No MQTT data received:**
- Check WiFi credentials in `arduino_secrets.h`
- Verify MQTT broker connectivity
- Ensure serial connection between Arduino and ESP32

**Incorrect wind measurements:**
- Recalibrate sensors in known conditions
- Check transducer positioning (East/West alignment)
- Verify analog pin connections

**7-segment display not working:**
- Check pin connections match code configuration
- Verify common cathode display type
- Test with simple digit display first

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with hardware setup
5. Submit a pull request

## License

This project is open source. See individual files for specific licensing information.

## Acknowledgments

This project uses:
- PlatformIO for embedded development
- PubSubClient library for MQTT communication
- ArduinoJson for data formatting
- Python scientific stack for data analysis
- Claude Sonnet 4 was used in the following capacities on the Python scripts:
   - Merging CSV files
   - Cleaning up the output of scripts
   - Conducting ANOVA tests

---

For detailed component-specific instructions, see the README files in each subfolder.