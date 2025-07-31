// Sonic Anemometer

#include <math.h>
#include <Arduino.h>

#include "sevSeg.hpp"

#define ADC_DELAY_US 100
#define MOVING_AVERAGE_SIZE 50
#define DISPLAY_UPDATE_RATE 1000

#define WEST_PIN A0
#define EAST_PIN A1

typedef struct{
  int windSpeed; // Wind speed in m/s
  int windDirection; // Wind direction in degrees
} WindData_t;

typedef struct{
  int W;
  int E;
} Wind_t;

// Calibration values for the sensors in still air
                      // W  , E
const Wind_t calm =   {547, 581}; // Calibration values for still air
const Wind_t north =  {850, 226}; // Calibration values for north wind
const Wind_t east =   {720, 712}; // Calibration values for east wind

const float windspeed = 10;   //Wind speed during calibration

// Define the pins for the 7-segment display
sevSeg myDisplay;
uint8_t digitPins[4] = {3, 2, 13, 12}; // Pins for the digits
uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11}; // Pins for the segments

WindData_t windCalculations(int reading, bool sensorSelect);
 
void setup()
{
  Serial.begin(9600);
  pinMode(WEST_PIN, INPUT);
  pinMode(EAST_PIN, INPUT);
  
  Serial.println("Sonic Anemometer");

  delay(1000);

  myDisplay.begin(digitPins, segmentPins); // Initialize the 7-segment display

  // Check if calibration values are zero
  if(north.W == calm.W || north.E == calm.E || east.W == calm.W || east.W == calm.E) {
    Serial.println("Error: Calibration values cannot be zero, check calibration values.");
    return; // Exit if calibration values are zero
  }
}
void loop()
{
  static unsigned long lastReading = millis();
  static unsigned long lastUpdate = millis();
  static int reading = 0; // int to hold the voltages from the sensors
  static bool sensorSelect = false; // Switch to alternate between east and west sensor readings
  // false = east sensor, true = west sensor

  static WindData_t windData; // Structure to hold wind data

  // Check which sensor to read based on the switch
  // This is because analog read takes 112us, so this reduces the time between readings
  if(millis() - lastReading >= ADC_DELAY_US){
    lastReading = millis(); // Update the last reading time
    reading = (sensorSelect) ? analogRead(WEST_PIN) : analogRead(EAST_PIN); // Measure sensor voltage
    windData = windCalculations(reading, sensorSelect); // Perform wind calculations
    sensorSelect = !sensorSelect; // Switch to the other sensor for the next reading
  } 

  // Update the value at the Display
  if(millis() - lastUpdate >= DISPLAY_UPDATE_RATE){
    myDisplay.setNumber(windData.windSpeed);
    lastUpdate = millis(); // Update the last update time
  }

  myDisplay.refreshDisplayNumber(); // Update the display with the new number
}

WindData_t windCalculations(int reading, bool sensorSelect)
{
  static int eastAvgArray[MOVING_AVERAGE_SIZE] = {0}; // Array to store east sensor readings
  static int westAvgArray[MOVING_AVERAGE_SIZE] = {0}; // Array to store west sensor readings

  static int eastAvgIndex = 0; // Index for east sensor readings
  static int westAvgIndex = 0; // Index for west sensor readings

  static float eastSum = 0; // Sum values for east and west sensors
  static float westSum = 0; // Sum values for east and west sensors

  WindData_t windData; // Structure to hold wind data

  // Rolling Average Calculation
  if(sensorSelect){
    // West Sensor Update
    westSum += reading;                                       // Add current west sensor reading to sum
    westSum -= westAvgArray[westAvgIndex];                    // Subtract the oldest west sensor reading from sum
    westAvgArray[westAvgIndex] = reading;                     // Store the current west sensor reading
    westAvgIndex = (westAvgIndex + 1) % MOVING_AVERAGE_SIZE;  // Update index for west sensor
  }
  else{
    // East Sensor Update
    eastSum += reading;                                       // Add current east sensor reading to sum
    eastSum -= eastAvgArray[eastAvgIndex];                    // Subtract the oldest east sensor reading from sum
    eastAvgArray[eastAvgIndex] = reading;                     // Store the current east sensor reading
    eastAvgIndex = (eastAvgIndex + 1) % MOVING_AVERAGE_SIZE;  // Update index for east sensor
  }

  // Serial.print("West sensor average: ");
  // Serial.println(westSum / MOVING_AVERAGE_SIZE);
  // Serial.print("East sensor average: ");
  // Serial.println(eastSum / MOVING_AVERAGE_SIZE);

  // Calculate West and East Transducer vectors
  float NW_WS_W = windspeed * (((westSum/MOVING_AVERAGE_SIZE) - calm.W) / (north.W - calm.W)); // North wind vector at West Transducer
  float NW_WS_E = windspeed * (((eastSum/MOVING_AVERAGE_SIZE) - calm.E) / (north.E - calm.E)); // North wind vector at East Transducer
  float EW_WS_W = windspeed * (((westSum/MOVING_AVERAGE_SIZE) - calm.W) / (east.W - calm.W)); // East wind vector at West Transducer
  float EW_WS_E = windspeed * (((eastSum/MOVING_AVERAGE_SIZE) - calm.E) / (east.E - calm.E)); // East wind vector at East Transducer

  float northWind = NW_WS_W + NW_WS_E; // Calculate the north wind vector
  float eastWind = EW_WS_W + EW_WS_E; // Calculate the east wind vector

  windData.windSpeed = round(sqrt((northWind * northWind) + (eastWind * eastWind)) / 2); // Calculate the wind speed

  windData.windDirection = (180 * (atan2(northWind, eastWind))) / PI; //270 - round(atan2(northWind, eastWind) * 57.3);//*(northwind<0);
  
  if (windData.windDirection > 359){
    windData.windDirection = windData.windDirection - 360; //Calculate wind direction
  }
  
  // Serial.print("Wind speed ");
  // Serial.print(windData.windSpeed);
  // Serial.println(" m/s");
  Serial.print("Wind direction " );
  Serial.print(windData.windDirection);
  Serial.println(" degrees");

  return windData; // Return the wind data structure
}