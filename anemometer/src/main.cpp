// Sonic Anemometer

#include <math.h>
#include <Arduino.h>

#include "sevSeg.hpp"

#define ADC_DELAY_US 112
#define MOVING_AVERAGE_SIZE 20
#define DISPLAY_UPDATE_RATE 1000

typedef struct{
  int windSpeed; // Wind speed in m/s
  int windDirection; // Wind direction in degrees
} WindData;

const int Wcalm = 575;      //West sensor reading in still air
const int Ecalm = 548;      //East sensor reading in still air
const int Wnorth = 847;     //West sensor reading in north wind
const int Enorth = 215;     //East sensor reading in north wind
const int Weast =576;       //West sensor reading in east wind
const int Eeast = 356;      //East sensor reading in east wind
const float windspeed = 9;   //Wind speed during calibration

const int west = A0;        //West sensor analog pin
const int east = A1;        //East sensor analog pin

// Define the pins for the 7-segment display
sevSeg myDisplay;
uint8_t digitPins[4] = {3, 2, 13, 12}; // Pins for the digits
uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11}; // Pins for the segments

WindData windCalculations(int Vwest, int Veast);
 
void setup()
{
  Serial.begin(9600);
  pinMode(west, INPUT);
  pinMode(east, INPUT);
  
  Serial.println("Sonic Anemometer");

  delay(1000);

  myDisplay.begin(digitPins, segmentPins); // Initialize the 7-segment display
}
void loop()
{
  static unsigned long lastADCReading = micros();
  static unsigned long lastUpdate = millis();
  static int Vwest = 0;
  static int Veast = 0;
  static bool eastWestSwitch = false; // Switch to alternate between east and west sensor readings

  static WindData windData; // Structure to hold wind data

  // Update the ADC reading
  if(micros() - lastADCReading >= ADC_DELAY_US) {

    // Check which sensor to read based on the switch
    if(eastWestSwitch){
      Vwest = analogRead(west);        // Measure west sensor voltage
    }
    else{
      Veast = analogRead(east);        //Measure east sensor voltage
    }

    eastWestSwitch = !eastWestSwitch; // Switch to the other sensor for the next reading
    lastADCReading = micros(); // Update the last ADC reading time
    
    windData = windCalculations(Vwest, Veast); // Perform wind calculations
  }

  // Update the value at the Display
  if(millis() - lastUpdate >= DISPLAY_UPDATE_RATE){
    myDisplay.setNumber(windData.windSpeed);
    lastUpdate = millis(); // Update the last update time
  }

  myDisplay.refreshDisplayNumber(); // Update the display with the new number
}

WindData windCalculations(int Vwest, int Veast)
{
  static int eastAvgArray[MOVING_AVERAGE_SIZE] = {0}; // Array to store east sensor readings
  static int westAvgArray[MOVING_AVERAGE_SIZE] = {0}; // Array to store west sensor readings

  static int eastAvgIndex = 0; // Index for east sensor readings
  static int westAvgIndex = 0; // Index for west sensor readings

  static int eastSum = 0; // Sum values for east and west sensors
  static int westSum = 0; // Sum values for east and west sensors

  WindData windData; // Structure to hold wind data

  // Serial.println(" ");
  // Serial.print("West sensor reading ");
  // Serial.println(Vwest);
  // Serial.print("East sensor reading ");
  // Serial.println(Veast);

  double northwind_denominator = (Wnorth-Wcalm) + (Enorth-Ecalm);
  if(northwind_denominator == 0) {
    Serial.println("Error: North wind denominator is zero, check calibration values.");
    northwind_denominator = 0.0001; // Avoid division by zero
  }

  double eastwind_denominator = (Eeast - Ecalm) - (Weast - Wcalm);
  if(eastwind_denominator == 0) {
    Serial.println("Error: East wind denominator is zero, check calibration values.");
    eastwind_denominator = 0.0001; // Avoid division by zero
  }

  // Rolling Average
  eastSum += Veast; // Add current east sensor reading to sum
  westSum += Vwest; // Add current west sensor reading to sum

  eastSum -= eastAvgArray[eastAvgIndex]; // Subtract the oldest east sensor reading from sum
  westSum -= westAvgArray[westAvgIndex]; // Subtract the oldest west sensor reading from sum

  eastAvgArray[eastAvgIndex] = Veast; // Store the current east sensor reading
  westAvgArray[westAvgIndex] = Vwest; // Store the current west sensor reading

  eastAvgIndex = (eastAvgIndex + 1) % MOVING_AVERAGE_SIZE; // Update index for east sensor
  westAvgIndex = (westAvgIndex + 1) % MOVING_AVERAGE_SIZE; // Update index for west sensor

  // Serial.print("West sensor average: ");
  // Serial.println(westSum / MOVING_AVERAGE_SIZE);
  // Serial.print("East sensor average: ");
  // Serial.println(eastSum / MOVING_AVERAGE_SIZE);

  double northwind = (((westSum/MOVING_AVERAGE_SIZE)-Wcalm) + ((eastSum/MOVING_AVERAGE_SIZE)-Ecalm))/(northwind_denominator)*windspeed; 
  double eastwind = ((westSum/MOVING_AVERAGE_SIZE)-Wcalm-((eastSum/MOVING_AVERAGE_SIZE)-Ecalm))/(eastwind_denominator)*windspeed;

  // Serial.print("North wind ");
  // Serial.print(northwind);
  // Serial.println(" m/s");
  // Serial.print("East wind ");
  // Serial.print(eastwind);
  // Serial.println(" m/s");
  
  windData.windSpeed = round(sqrt(northwind*northwind+eastwind*eastwind)); //Calculate wind speed

  windData.windDirection = 270-round(atan2(northwind, eastwind)*57.3);//*(northwind<0);
  
  if (windData.windDirection>359){
    windData.windDirection=windData.windDirection-360; //Calculate wind direction
  }
  
  // Serial.print("Wind speed ");
  // Serial.print(windData.windSpeed);
  // Serial.println(" m/s");
  // Serial.print("Wind direction " );
  // Serial.print(heading);
  // Serial.println(" degrees");

  return windData; // Return the wind data structure
}
