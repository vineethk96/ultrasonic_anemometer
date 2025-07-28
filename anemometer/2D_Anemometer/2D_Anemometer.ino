// Sonic Anemometer

#include <math.h>
#include <Arduino.h>
  
const int Wcalm = 540;      //West sensor reading in still air
const int Ecalm = 540;      //East sensor reading in still air
const int Wnorth = 540;     //West sensor reading in north wind
const int Enorth = 540;     //East sensor reading in north wind
const int Weast =530;       //West sensor reading in east wind
const int Eeast = 500;      //East sensor reading in east wind
const float windspeed = 5;   //Wind speed during calibration

const int west = A0;        //West sensor analog pin
const int east = A1;        //East sensor analog pin
 
void setup()
{
   Serial.begin(9600);
    pinMode(west, INPUT);
    pinMode(east, INPUT);
    delay(100);
    }
void loop()
{
  int Vwest = analogRead(west);        // Measure west sensor voltage
  Vwest = analogRead(west);        // Repeat for settling time
  int Veast = analogRead(east);        //Measure east sensor voltage
  Veast = analogRead(east);
  Serial.println(" ");
  Serial.print("West sensor reading ");
  Serial.println(Vwest);
  Serial.print("East sensor reading ");
  Serial.println(Veast);

  double northwind = (Vwest+Veast-Ecalm-Wcalm)/(Wnorth+Enorth-Ecalm-Wcalm)*windspeed; 
  double eastwind = (Vwest-Wcalm-(Veast-Ecalm))/(Weast-Wcalm-(Eeast-Ecalm))*windspeed;
  
  int wind =round(sqrt(northwind*northwind+eastwind*eastwind)); //Calculate wind speed
  int heading=270-round(atan2((double)-northwind,(double)-eastwind)*57.3);//*(northwind<0);
  if (heading>359)heading=heading-360; //Calculate wind direction
  Serial.print("Wind speed ");
  Serial.print(wind);
  Serial.println(" m/s");
  Serial.print("Wind direction " );
  Serial.print(heading);
  Serial.println(" degrees");
  delay(3000);
}