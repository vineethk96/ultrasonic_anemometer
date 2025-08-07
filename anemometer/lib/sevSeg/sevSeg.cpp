#include <Arduino.h>
#include "sevSeg.hpp"

#define refreshRate 1 // Refresh rate in milliseconds

uint8_t zero[8] =   {0, 0, 1, 0, 1, 0, 0, 0};
uint8_t one[8] =    {1, 1, 1, 0, 1, 1, 1, 0};
uint8_t two[8] =    {0, 0, 1, 1, 0, 0, 1, 0};
uint8_t three[8] =  {1, 0, 1, 0, 0, 0, 1, 0};
uint8_t four[8] =   {1, 1, 1, 0, 0, 1, 0, 0};
uint8_t five[8] =   {1, 0, 1, 0, 0, 0, 0, 1};
uint8_t six[8] =    {0, 0, 1, 0, 0, 0, 0, 1};
uint8_t seven[8] =  {1, 1, 1, 0, 1, 0, 1, 0};
uint8_t eight[8] =  {0, 0, 1, 0, 0, 0, 0, 0};
uint8_t nine[8] =   {1, 0, 1, 0, 0, 0, 0, 0};

void sevSeg::displayDigit(uint8_t index, uint8_t digit)
{
    if (index > 3) {
        Serial.println("Error: Index out of range (0-3)");
        return;
    }

    uint8_t* numberArray = nullptr;

    switch(digit) {
        case 0: numberArray = zero; break;
        case 1: numberArray = one; break;
        case 2: numberArray = two; break;
        case 3: numberArray = three; break;
        case 4: numberArray = four; break;
        case 5: numberArray = five; break;
        case 6: numberArray = six; break;
        case 7: numberArray = seven; break;
        case 8: numberArray = eight; break;
        case 9: numberArray = nine; break;
        default:
            Serial.println("Error: Invalid digit (0-9 expected)");
            return;
    }

    // Clear all pins first
    clearPins();

    digitalWrite(digitPinArray[index], LOW); // Enable this digit (LOW = on for common cathode)
    
    // Set segments for this digit
    for(uint8_t i = 0; i < 8; i++) {
        digitalWrite(segPinArray[i], numberArray[i]);
    }
    
    digitalWrite(digitPinArray[index], HIGH);
    
    // // Keep digit on for longer (multiplexing delay)
    // delay(5);  // Increased delay for visibility
}

void sevSeg::clearPins(void)
{
    // Turn off all segments (HIGH = off for common anode)
    for(uint8_t j = 0; j < 8; j++) {
        digitalWrite(segPinArray[j], HIGH);
    }

    // Turn off all digits (LOW = off for common anode)
    for(uint8_t i = 0; i < 4; i++) {
        digitalWrite(digitPinArray[i], LOW);
    }
}

sevSeg::sevSeg(void)
{
}

void sevSeg::begin(uint8_t digitPins[4], uint8_t segmentPins[8])
{
    digitPinArray = digitPins;
    segPinArray = segmentPins;

    Serial.print("Initializing 7-segment display with digits: ");
    for (uint8_t i = 0; i < 4; i++) {
        Serial.print(digitPinArray[i]);
        if (i < 3) Serial.print(", ");
    }
    Serial.println();
    Serial.print("Segment pins: ");
    for (uint8_t i = 0; i < 8; i++) {
        Serial.print(segPinArray[i]);
        if (i < 7) Serial.print(", ");
    }
    Serial.println();
    
    // Set up digit pins
    for(uint8_t i = 0; i < 4; i++) {
        pinMode(digitPinArray[i], OUTPUT);
    }

    // Set up segment pins
    for(uint8_t i = 0; i < 8; i++) {
        pinMode(segPinArray[i], OUTPUT);
    }

    clearPins(); // Clear all pins initially
}

void sevSeg::setNumber(int number)
{
    if(number < 0 || number > 9999) {
        Serial.println("Error: Number out of range (0-9999)");
        return;
    }
    
    if(number == oldNum) {
        return; // No need to update if same number
    }
    
    oldNum = number;

    // Extract digits (rightmost digit goes to index 3)
    for(int i = 3; i >= 0; i--) {
        numberArray[i] = number % 10;
        number /= 10;
    }

    // //Serial.print("Display array: ");
    // for(int i = 0; i < 4; i++) {
    //     //Serial.print(numberArray[i]);
    //     //if(i < 3) Serial.print(", ");
    // }
    // //Serial.println();
}

void sevSeg::refreshDisplay(void)
{
    static uint8_t digitIndex = 0;
    static unsigned long lastUpdate = 0;
    
    // Update every 20ms for slower, more visible multiplexing
    if(millis() - lastUpdate >= refreshRate) {
        // Display the number stored in numberArray[digitIndex] at digit position digitIndex
        displayDigit(digitIndex, numberArray[digitIndex]);
        
        // Move to next digit position
        digitIndex = (digitIndex + 1) % 4;
        
        lastUpdate = millis();
    }
}

void sevSeg::refreshDisplayNumber(void)
{
    static uint8_t digitIndex = 0;
    static unsigned long lastUpdate = millis();

    if(millis() - lastUpdate >= refreshRate){

        uint8_t* segArray = nullptr;

        switch(digitIndex){
            case 0:{
                // If the first index is 0, we don't want to display it
                if(numberArray[0] == 0){
                    digitIndex = (digitIndex + 1) % 4; // Move to next digit position
                }
                else break; // If first index is not 0, we can display it
            }
            case 1:{
                // If the first index is 0, and the second index is also 0, we don't want to display it
                if(numberArray[0] == 0 && numberArray[1] == 0){
                    digitIndex = (digitIndex + 1) % 4; // Move to next digit position
                }
                else break; // If first index is not 0, we can display it
            }
            case 2:{
                // If the first index is 0, and the second index is also 0, and the third index is also 0, we don't want to display it
                if(numberArray[0] == 0 && numberArray[1] == 0 && numberArray[2] == 0){
                    digitIndex = (digitIndex + 1) % 4; // Move to next digit position
                }
                else break; // If first index is not 0, we can display it
            }
            case 3:
                break; // Last digit, no check needed
        }

        switch(numberArray[digitIndex]) {
            case 0: segArray = zero; break;
            case 1: segArray = one; break;
            case 2: segArray = two; break;
            case 3: segArray = three; break;
            case 4: segArray = four; break;
            case 5: segArray = five; break;
            case 6: segArray = six; break;
            case 7: segArray = seven; break;
            case 8: segArray = eight; break;
            case 9: segArray = nine; break;
            default:
                Serial.println("Error: Invalid digit (0-9 expected)");
                return;
        }
    
        // Clear all pins first
        clearPins();
    
        digitalWrite(digitPinArray[digitIndex], LOW); // Enable this digit (LOW = on for common cathode)
        
        // Set segments for this digit
        for(uint8_t i = 0; i < 8; i++) {
            digitalWrite(segPinArray[i], segArray[i]);
        }
        
        digitalWrite(digitPinArray[digitIndex], HIGH);

        // Move to the next digit, and update the timestamp
        digitIndex = (digitIndex + 1) % 4; // Move to next digit position
        lastUpdate = millis();

    }
}
