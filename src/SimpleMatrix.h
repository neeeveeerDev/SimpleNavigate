#ifndef SIMPLE_MATRIX_H
#define SIMPLE_MATRIX_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "SimpleButton.h"

class SimpleMatrix {
public:
    using KeyCallback = std::function<void(char key)>;

    SimpleMatrix(const std::vector<uint8_t>& rowPins, const std::vector<uint8_t>& colPins, const std::vector<std::vector<char>>& layout);
    ~SimpleMatrix();

    static void tickAll();

    void tick();

    void onKeyPress(KeyCallback callback);
    void onKeyRelease(KeyCallback callback);

    SimpleButton& key(char keyChar);
    bool isPressed(char keyChar) const;
    char readKey();

private:
    friend class SimpleNavigate;

    struct KeyEntry {
        char keyChar;
        uint8_t row;
        uint8_t col;
        bool isDown;
        SimpleButton* button;
    };

    std::vector<uint8_t> _rowPins;
    std::vector<uint8_t> _colPins;
    bool _configured;

    std::vector<KeyEntry> _keys;
    SimpleButton _dummyButton;

    char _lastPressedKey;
    KeyCallback _onKeyPress;
    KeyCallback _onKeyRelease;

    static std::vector<SimpleMatrix*> _instances;

    void _scan();
};

#endif
