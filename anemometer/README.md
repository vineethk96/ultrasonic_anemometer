# Anemometer - Wind Measurement Module

This folder contains the Arduino code for the ultrasonic anemometer that calculates 2D wind speed and direction measurements.

## Overview

The anemometer uses three ultrasonic transducers positioned North, East, and West to measure wind speed and direction through phase-shift calculations. Wind data is displayed on a 7-segment display and transmitted via serial to the WiFi module.

## Hardware Requirements

- **Arduino Uno**
- **3x RX Ultrasonic Transducers** (positioned North, East, West)
- **1x TX Ultrasonic Transducer** 
- **4-digit 7-segment display** (common cathode)
- **Resistors** (220Ω recommended for segment limiting)
- **Various other electrical components**

## Pin Configuration

### Analog Inputs
- `A0` - West ultrasonic transducer
- `A1` - East ultrasonic transducer
- North RX Transducer is phase-locked to TX

### 7-Segment Display (Common Cathode)
#### Digit Control Pins:
- `Pin 3` - Digit 1 (leftmost)
- `Pin 2` - Digit 2  
- `Pin 13` - Digit 3
- `Pin 12` - Digit 4 (rightmost)

#### Segment Control Pins (a, b, c, d, e, f, g, dp):
- `Pin 4` - Segment A
- `Pin 5` - Segment B
- `Pin 6` - Segment C
- `Pin 7` - Segment D
- `Pin 8` - Segment E
- `Pin 9` - Segment F
- `Pin 10` - Segment G
- `Pin 11` - Decimal Point

## Software Setup

### 1. Install PlatformIO
Install the PlatformIO extension in VS Code or use PlatformIO Core.

### 2. Open Project
Open the `anemometer` folder in PlatformIO.

### 3. Select Environment
The `platformio.ini` file defines two environments:
- `uno` - for Arduino Uno

Select the appropriate environment in PlatformIO.

### 4. Upload Code
```bash
pio run --target upload --environment uno
```

## Calibration Process

The anemometer requires calibration in three wind conditions. Update these values in `src/main.cpp`:

```cpp
// Calibration values for the sensors in still air
const Wind_t calm =   {547, 581}; // W, E sensor readings in calm air
const Wind_t north =  {850, 226}; // W, E sensor readings in 10 m/s north wind  
const Wind_t east =   {720, 712}; // W, E sensor readings in 10 m/s east wind
```

### Calibration Steps:
1. **Calm Air Calibration:**
   - Place anemometer in still air conditions
   - Record average readings from both sensors
   - Update `calm` values

2. **North Wind Calibration:**
   - Generate known 9 m/s wind from north direction
   - Record sensor readings
   - Update `north` values

3. **East Wind Calibration:**
   - Generate known 9 m/s wind from east direction
   - Record sensor readings  
   - Update `east` values

## Code Structure

### Main Components

#### `main.cpp`
- **Wind Calculation Algorithm:** Implements 2D vector wind speed/direction calculation
- **Moving Average Filter:** 50-sample rolling average for noise reduction
- **Sensor Reading:** Alternates between East/West sensors every 100μs
- **Display Control:** Updates 7-segment display every second
- **Serial Output:** Sends JSON wind data via serial

#### Custom Library: `sevSeg`
Located in `lib/sevSeg/`, this library handles the 4-digit 7-segment display:

- **`sevSeg.hpp`** - Class definition and interface
- **`sevSeg.cpp`** - Implementation with digit encoding and multiplexing

### Key Parameters

```cpp
#define ADC_DELAY_US 100          // Delay between sensor readings (μs)
#define MOVING_AVERAGE_SIZE 50    // Rolling average window size
#define DISPLAY_UPDATE_RATE 1000  // Display refresh rate (ms)
```

### Data Structures

```cpp
typedef struct{
  float windSpeed;     // Wind speed in m/s
  int windDirection;   // Wind direction in degrees (0-360°)
} WindData_t;

typedef struct{
  int W;  // West sensor reading
  int E;  // East sensor reading  
} Wind_t;
```

## Algorithm Details

### Wind Calculation Process
1. **Sensor Reading:** Alternates between East/West transducers
2. **Moving Average:** Maintains 50-sample rolling average for each sensor
3. **Vector Calculation:** 
   - Calculates North and East wind components using calibration data
   - Uses linear interpolation between calm and reference wind conditions
4. **Wind Speed:** `sqrt(north² + east²) / 2`
5. **Wind Direction:** `atan2(north, east)` converted to 0-360° meteorological convention

### Output Format
```json
{"speed":5.50,"direction":126}
```

Serial output at 9600 baud for transmission to WiFi module.

## Display Features

- **Wind Speed Display:** Shows current wind speed in m/s (integer value)
- **Multiplexed Display:** Efficient 4-digit display with 1ms refresh rate
- **Leading Zero Suppression:** Hides unnecessary leading zeros
- **Range:** Displays 0-9999 (wind speeds over 999 m/s show as 9999)

## Troubleshooting

### No Display Output
- Check 7-segment display wiring
- Verify common cathode display type
- Test individual segments with simple code

### Incorrect Wind Readings
- Recalibrate sensors with known wind conditions
- Check transducer alignment (ensure East-West positioning)
- Verify analog input connections

### Erratic Readings
- Increase moving average window size
- Check for electrical interference
- Ensure stable power supply

### Serial Communication Issues
- Verify 9600 baud rate setting
- Check serial connection to ESP32
- Monitor serial output with Arduino IDE Serial Monitor

## Monitoring and Testing

### Serial Monitor Output
Connect to the Arduino's serial port at 9600 baud to see:
- JSON wind data: `{"speed":X.XX,"direction":XXX}`
- Calibration verification data
- Error messages and diagnostics

### Testing Procedure
1. Upload code to Arduino
2. Open Serial Monitor (9600 baud)
3. Verify JSON output format
4. Test display with known wind conditions
5. Validate readings against reference measurements

## Performance Specifications

- **Update Rate:** Wind calculations every 200μs (5 kHz)
- **Display Update:** 1 Hz (1 second intervals)
- **Serial Output:** Continuous JSON stream
- **Accuracy:** Depends on calibration quality and sensor positioning
- **Range:** 0-999 m/s (display limitation)