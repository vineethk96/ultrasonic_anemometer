# Data Collector - Python Analysis Tools

This folder contains Python scripts for collecting, processing, and analyzing wind data from the ultrasonic anemometer system.

## Overview

The data collector suite provides three main tools:
1. **MQTT Collector** - Real-time data collection from MQTT broker
2. **CSV Merger** - Combines multiple data files for analysis
3. **Statistical Analysis** - Comprehensive ANOVA and effect size analysis

## Requirements

### Python Version
- Python 3.7 or higher

### Required Packages
Create a Virtual Environment:
```bash
python -m venv analysis_env
```

Start Virtual Environment:
```bash
analysis_env\Scripts\activate
```

Install the required packages using pip:

```bash
pip install paho-mqtt pandas numpy statsmodels scipy
```

Or install from requirements file:
```bash
pip install -r requirements.txt
```

**_NOTE:_** Remember to deactivate Virtual Environment after use
```bash
deactivate
```

#### Package Details:
- `paho-mqtt>=1.6.1` - MQTT client for data collection
- `pandas>=1.3.0` - Data manipulation and analysis
- `numpy>=1.21.0` - Numerical computations
- `statsmodels>=0.12.0` - Statistical modeling and ANOVA
- `scipy>=1.7.0` - Scientific computing and statistics

## Tools Overview

### 1. MQTT Collector (`mqtt_collector.py`)

Connects to MQTT broker and collects 200 wind readings in real-time.

#### Usage:
```bash
python mqtt_collector.py
```

#### Interactive Setup:
1. **CSV Filename:** Enter desired output filename (`.csv` added automatically)
2. **MQTT Topic:** Enter the topic to subscribe to (e.g., `student/ultrasonic_anemometer`)
3. **Confirmation:** Review settings and confirm collection

#### Features:
- **Real-time Collection:** Streams live data from MQTT broker
- **Format Validation:** Parses JSON format `{"speed":X.XX,"direction":XXX}`
- **Progress Tracking:** Shows collection progress (X/200 readings)
- **Automatic Statistics:** Calculates mean, min, max, std deviation
- **Error Handling:** Graceful handling of connection issues
- **Interrupt Support:** Ctrl+C saves partial data

#### Output Format:
CSV file with columns:
- `timestamp` - ISO format timestamp
- `topic` - MQTT topic name
- `wind_speed` - Wind speed in m/s
- `wind_direction` - Wind direction in degrees

#### Example Output:
```csv
timestamp,topic,wind_speed,wind_direction
2024-01-15T14:30:15.123456,student/ultrasonic_anemometer,5.50,126
2024-01-15T14:30:16.234567,student/ultrasonic_anemometer,5.75,128
```

### 2. CSV Merger (`merger.py`)

Combines multiple CSV files into a single summary file for batch analysis.

#### Usage:
```bash
python merger.py
```

#### Interactive Process:
1. **Directory Path:** Enter path containing CSV files
2. **File Discovery:** Automatically finds all `.csv` files
3. **Processing:** Extracts wind_speed data from each file
4. **Output:** Creates `wind_speed_summary.csv`

#### Features:
- **Batch Processing:** Handles multiple CSV files automatically
- **Column Validation:** Verifies required columns exist
- **Filename Preservation:** Creates unique column names from filenames
- **Error Handling:** Skips invalid files with warnings
- **Data Preview:** Shows first few rows of combined data

#### Output Structure:
```csv
index,file1_wind_speed,file2_wind_speed,file3_wind_speed
1,5.50,6.20,4.80
2,5.75,6.45,4.95
```

### 3. Statistical Analysis (`analysis.py`)

Performs comprehensive statistical analysis on wind speed data collected under different experimental conditions.

#### Usage:
```bash
python analysis.py
```

#### Required Data Structure:
The script expects CSV files organized by experimental conditions in a `readings/` directory:

```
readings/
├── aboveTreeLevel/
│   ├── open.csv
│   ├── half_building.csv
│   ├── closed_park.csv
│   └── ...
├── treeLevel/
│   └── ...
└── groundLevel/
    └── ...
```

#### Experimental Factors:
- **Height:** `aboveTreeLevel`, `treeLevel`, `groundLevel`
- **Width:** `open`, `half`, `closed`  
- **Obstacles:** `none`, `building`, `park`

#### Statistical Methods:

##### 1. Two-Way ANOVA (by height level)
- **Factors:** Width × Obstacles
- **Response:** Wind speed
- **Type:** Type II ANOVA (balanced designs)

##### 2. Three-Way ANOVA (all data)
- **Factors:** Height × Width × Obstacles
- **Includes:** All main effects and interactions

##### 3. Effect Size Analysis
- **Omega Squared (ω²):** Less biased than eta squared
- **Interpretation:** 
  - < 0.01: negligible
  - 0.01-0.06: small
  - 0.06-0.14: medium  
  - ≥ 0.14: large

##### 4. Specific Contrasts
- **Obstacle Effects:** Building vs None, Park vs None
- **Confidence Intervals:** 95% CI for effect differences
- **Interaction Analysis:** Effects by height level

#### Output Features:
- **Data Quality Checks:** Sample sizes, missing data, descriptive stats
- **ANOVA Tables:** F-statistics, p-values, degrees of freedom
- **Effect Sizes:** Omega squared with magnitude interpretation
- **Contrast Analysis:** Specific comparisons of interest
- **Summary Statistics:** Overall dataset characteristics

#### Example Output:
```
TWO-WAY ANOVA ANALYSES BY HEIGHT
================================

Two-Way ANOVA for aboveTreeLevel
================================
                    sum_sq    df         F    PR(>F)
C(width)            45.23     2    8.756   0.0003
C(obstacles)        12.34     2    2.389   0.0956
C(width):C(obstacles) 3.45   4    0.334   0.8551
Residual           234.56    91

Omega Squared (O_sq) Effect Sizes:
-----------------------------------
C(width)                      : O_sq = 0.1234 (medium)
C(obstacles)                  : O_sq = 0.0234 (small)
C(width):C(obstacles)         : O_sq = 0.0000 (negligible)
```

## Workflow Example

### Complete Data Collection and Analysis Workflow:

1. **Collect Data:**
   ```bash
   python mqtt_collector.py
   # Collect data for each experimental condition
   # Example: open_none_height1.csv, open_building_height1.csv, etc.
   ```

2. **Organize Data:**
   ```
   mkdir -p readings/aboveTreeLevel readings/treeLevel readings/groundLevel
   # Move CSV files to appropriate directories
   # Rename files to match expected format (width_obstacle.csv)
   ```

3. **Optional - Merge Multiple Files:**
   ```bash
   python merger.py
   # If you have multiple files per condition to combine
   ```

4. **Run Analysis:**
   ```bash
   python analysis.py
   # Performs complete statistical analysis
   ```

## Data Quality Requirements

### File Naming Convention:
- Format: `{width}_{obstacle}.csv` (e.g., `open_building.csv`)
- Width options: `open`, `half`, `closed`
- Obstacle options: `none`, `building`, `park`
- Special case: Files with just width name (e.g., `open.csv`) assume `obstacle = none`

### CSV Structure:
Required columns in each CSV file:
- `wind_speed` - Numerical wind speed values
- Other columns (timestamp, topic, wind_direction) are preserved but not required for analysis

### Sample Size Considerations:
- **Minimum:** 30 readings per condition for reliable statistics
- **Recommended:** 200 readings per condition for robust analysis
- **Balance:** Equal sample sizes across conditions improve ANOVA reliability

## Troubleshooting

### MQTT Collection Issues:

**Connection Failed:**
```
Failed to connect to MQTT broker. Return code: X
```
- Check internet connection
- Verify MQTT broker address (`mqtt.cetools.org`)
- Test with MQTT client tools

**No Data Received:**
```
Skipped invalid format: some_message
```
- Check ESP32 is publishing to correct topic
- Verify data format: `{"speed":X.XX,"direction":XXX}`
- Monitor Arduino serial output

**Incomplete Collection:**
- Use Ctrl+C to save partial data
- Check WiFi stability during collection
- Restart collection if needed

### Analysis Issues:

**File Not Found:**
```
FileNotFoundError: readings/ directory not found
```
- Create `readings/` directory structure
- Move CSV files to appropriate subdirectories
- Check file naming convention

**Missing Columns:**
```
KeyError: 'wind_speed' column not found
```
- Verify CSV files have `wind_speed` column
- Check column names match exactly
- Review CSV file structure

**Insufficient Data:**
```
Warning: Small sample size in condition X
```
- Collect more data for affected conditions
- Consider combining similar conditions
- Check for missing experimental conditions

### CSV Merger Issues:

**No CSV Files Found:**
```
No CSV files found in directory 'path'
```
- Check directory path is correct
- Ensure files have `.csv` extension
- Verify file permissions

**Column Validation Failed:**
```
Warning: file.csv missing required columns. Skipping.
```
- Check CSV file structure
- Ensure required columns exist: `timestamp`, `topic`, `wind_speed`, `wind_direction`
- Review file formatting

## Performance Notes

- **MQTT Collection:** ~1-5 readings/second depending on transmission rate
- **CSV Merger:** Handles files up to 1M+ rows efficiently
- **Analysis:** Processing time scales with dataset size, typically <30 seconds for normal datasets

## Advanced Configuration

### MQTT Broker Settings:
Modify `mqtt_collector.py` to use different brokers:
```python
collector = MQTTDataCollector("your.mqtt.broker.com", port=1883)
```

### Analysis Customization:
Modify `analysis.py` for different experimental designs:
- Change factor names