#ifndef SEVSEG_HPP
#define SEVSEG_HPP

#include <Arduino.h>

class sevSeg {
    private:
        uint8_t* digitPinArray;
        uint8_t* segPinArray;

        int numberArray[4] = {0,0,0,0};
        int oldNum = 0;

        void setPin(int pin, bool state);
        void displayDigit(uint8_t index, uint8_t digit);
        void displayDecimalPoint();
        void clearPins(void);
    public:
        sevSeg(void);
        void begin(uint8_t digitPins[4], uint8_t segmentPins[8]);
        void setNumber(int number);
        void refreshDisplay(void);
        void refreshDisplayNumber(void);
};

#endif