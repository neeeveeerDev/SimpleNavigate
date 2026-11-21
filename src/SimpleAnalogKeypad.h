#ifndef SIMPLE_ANALOG_KEYPAD_H
#define SIMPLE_ANALOG_KEYPAD_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "SimpleButton.h"

class SimpleAnalogKeypad {
public:
    using KeyCallback = std::function<void(char id)>;

    SimpleAnalogKeypad(uint8_t analogPin, uint16_t tolerance = 50);
    ~SimpleAnalogKeypad();

    static void tickAll();

    void tick();

    void addKey(char id, uint16_t expectedAdc);
    void setTolerance(uint16_t tolerance);
    void setSmoothing(float factor);

    SimpleButton& key(char id);
    bool isPressed(char id) const;
    char readKey();
    uint16_t getRawAdc() const;

    void onKeyPress(KeyCallback callback);
    void onKeyRelease(KeyCallback callback);

private:
    friend class SimpleNavigate;

    struct AnalogKeyEntry {
        char id;
        uint16_t expectedAdc;
        bool isDown;
        SimpleButton* button;
    };

    uint8_t _pin;
    uint16_t _tolerance;
    float _smoothing;
    float _filteredAdc;

    std::vector<AnalogKeyEntry> _keys;
    SimpleButton _dummyButton;

    char _activeKey;
    char _lastPressedKey;

    KeyCallback _onKeyPress;
    KeyCallback _onKeyRelease;

    static std::vector<SimpleAnalogKeypad*> _instances;
};

#endif
