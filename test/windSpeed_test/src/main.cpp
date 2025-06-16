#include <Arduino.h>
#include <NewPing.h>

const int trigPin = 9; // Trigger pin for ultrasonic sensor
const int echoPin = 10; // Echo pin for ultrasonic sensor

unsigned long duration;
long windSpeed;
unsigned long pingStart;
const double distance = 0.2; // 20cm

NewPing sonar(trigPin, echoPin, 25);

void setup() {
  pinMode(trigPin, OUTPUT); // Set trigger pin as output
  pinMode(echoPin, INPUT); // Set echo pin as input
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  delay(1000); // Wait for serial to initialize
  Serial.println("Ultrasonic Sensor Test");
}

void loop() {

  pulseIn();
  unsigned long pingTime = sonar.ping_median(5);
  Serial.print("Ping Time (us): ");
  Serial.println(pingTime);
  // Convert ping time to distance in cm
  duration = pingTime * (0.000001);
  Serial.print("Ping Time: ");
  Serial.println(duration);
  windSpeed = (distance/duration) - 343;
  Serial.print("Wind Speed: ");
  Serial.print(windSpeed);
  Serial.println(" m/s");

  Serial.println("**************");
  delay(100);
}