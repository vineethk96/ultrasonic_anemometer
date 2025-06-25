#include <Arduino.h> // Include the ESP32 library for GPIO definitions

/* Pin Definitions */

// Controls PLL Inhibit Pin
#define enablePLL1_pin 17
#define enablePLL2_pin 30
#define enablePLL3_pin 31

// Controls the TX and RX Switches
  // TODO: Run the A, B, C signals to the same pins, to reduce the number of pins used.
#define txSwitchA_pin 9
#define txSwitchB_pin 10
#define txSwitchC_pin 11

#define rxSwitchA_pin 26
#define rxSwitchB_pin 27
#define rxSwitchC_pin 28

#define txSwitchEnable_pin 7
#define rxSwitchEnable_pin 8

// Reads from the ADC on the return path
#define signal1_pin 3
#define signal2_pin 4
#define signal3_pin 6

/* Logic Macros */

// When the Inhibit pin is grounded, the PLL sends out a clock signal.
// When the Inhibit pin is high, the PLL does not send out a clock signal.
#define enablePLL 0
#define disablePLL 1

// Defines the direction of the waveform
#define top2bottom 0
#define bottom2top 1

#define signalTime 125 // Time in microseconds to enable the PLL
#define stallTime 1000 // Time in microseconds to wait before starting a new signal

#define waveCount 5

enum States_t{
  WAITING,
  SENDING,
  POLLING,
};

States_t currentState = WAITING;

void setup() {

  Serial.begin(9600);

  /* Setup Pins */
  pinMode(enablePLL1_pin, OUTPUT);
  // pinMode(enablePLL2_pin, OUTPUT);
  // pinMode(enablePLL3_pin, OUTPUT);
  // pinMode(txSwitchA_pin, OUTPUT);
  // pinMode(txSwitchB_pin, OUTPUT);
  // pinMode(txSwitchC_pin, OUTPUT);
  // pinMode(rxSwitchA_pin, OUTPUT);
  // pinMode(rxSwitchB_pin, OUTPUT);
  // pinMode(rxSwitchC_pin, OUTPUT);
  // pinMode(txSwitchEnable_pin, OUTPUT);
  // pinMode(rxSwitchEnable_pin, OUTPUT);
  // pinMode(signal1_pin, INPUT);
  // pinMode(signal2_pin, INPUT);
  // pinMode(signal3_pin, INPUT);

  // /* Initialize Pins */
  // // Disable all pins to start
  //digitalWrite(enablePLL1_pin, disablePLL);
  // digitalWrite(enablePLL2_pin, disablePLL);
  // digitalWrite(enablePLL3_pin, disablePLL);

  // digitalWrite(txSwitchEnable_pin, LOW);
  // digitalWrite(rxSwitchEnable_pin, LOW);

  Serial.println("beginning...");
}

void loop() {

  static bool signalFlag = false;
  static unsigned long signalStartTime = 0;
  static unsigned long readTime = 0;
  static double maxArray[waveCount] = {0, 0, 0, 0, 0};
  static uint8_t waveIndex = 0;

  unsigned long loopStartTime = micros();

  // Serial.println(".");

  digitalWrite(enablePLL1_pin, enablePLL);
  delay(1000);
  digitalWrite(enablePLL1_pin, disablePLL);
  delay(1000);

  // switch(currentState){
  //   case WAITING:{

  //     // If the time elapsed has surpassed `stallTime`
  //     if(loopStartTime - micros() > stallTime){
  //       currentState = SENDING;
  //     }

  //     break;
  //   }
  //   case SENDING:{
  //     // Enable the PLLs
  //     //signalFlag = true;
  //     digitalWrite(enablePLL1_pin, enablePLL);
  //     signalStartTime = micros();

  //     currentState = POLLING;
  //     break;
  //   }
  //   case POLLING:{

  //     // read every microsecond
  //     if(readTime - micros() > 1){
  //       Serial.println(analogRead(signal1_pin));
  //     }


  //     break;
  //   }
  // }

}