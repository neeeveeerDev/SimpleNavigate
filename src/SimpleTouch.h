#ifndef SIMPLE_TOUCH_H
#define SIMPLE_TOUCH_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "SimpleButton.h"

class SimpleTouch {
public:
    SimpleTouch(uint8_t touchPin, uint16_t thresholdDelta = 15);
    ~SimpleTouch();

    static void tickAll();

    void tick();
    void calibrate();

    SimpleButton& btn();
    bool isTouched() const;
    uint16_t getRawValue() const;
    uint16_t getBaseline() const;
    void setThresholdDelta(uint16_t delta);

private:
    friend class SimpleNavigate;

    uint8_t _pin;
    uint16_t _thresholdDelta;
    uint16_t _baseline;
    uint16_t _rawValue;
    bool _calibrated;
    bool _isTouched;

    SimpleButton _button;

    static std::vector<SimpleTouch*> _instances;
};

#endif
