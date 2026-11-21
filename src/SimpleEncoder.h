#ifndef SIMPLE_ENCODER_H
#define SIMPLE_ENCODER_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "SimpleButton.h"

class SimpleEncoder {
public:
    using Callback = std::function<void()>;
    using ChangeCallback = std::function<void(int16_t delta, int16_t position)>;

    SimpleEncoder(uint8_t pinA, uint8_t pinB, int8_t pinBtn = -1, uint8_t activeLevel = LOW, uint8_t pinConfig = INPUT_PULLUP);
    ~SimpleEncoder();

    static void tickAll();

    void tick();

    void onLeft(Callback callback);
    void onRight(Callback callback);
    void onLeftHold(Callback callback);
    void onRightHold(Callback callback);
    void onChange(ChangeCallback callback);

    int16_t getValue() const;
    void setValue(int16_t val);
    int16_t getDelta();
    bool hasTurnedLeft();
    bool hasTurnedRight();
    bool isFast() const;

    void setRange(int16_t minVal, int16_t maxVal, bool cyclic = false);
    void clearRange();
    void setStep(int16_t step);
    void setStepsPerDetent(uint8_t steps);
    void setInverted(bool inverted);
    void setAcceleration(bool enabled, uint16_t fastThresholdMs = 50, uint8_t multiplier = 5);

    SimpleButton& btn();
    bool hasButton() const;

private:
    friend class SimpleNavigate;

    uint8_t _pinA;
    uint8_t _pinB;
    uint8_t _pinConfig;
    bool _configured;

    SimpleButton* _button;
    SimpleButton _dummyButton;

    uint8_t _lastState;
    int8_t _stepAccumulator;
    uint8_t _stepsPerDetent;
    bool _inverted;

    int16_t _value;
    int16_t _minValue;
    int16_t _maxValue;
    bool _hasRange;
    bool _cyclic;
    int16_t _step;

    bool _accelEnabled;
    uint16_t _fastThresholdMs;
    uint8_t _accelMultiplier;
    uint32_t _lastTurnTime;
    bool _isFast;

    bool _flagLeft;
    bool _flagRight;
    int16_t _deltaAccumulator;

    Callback _onLeft;
    Callback _onRight;
    Callback _onLeftHold;
    Callback _onRightHold;
    ChangeCallback _onChange;

    static std::vector<SimpleEncoder*> _instances;
};

#endif
