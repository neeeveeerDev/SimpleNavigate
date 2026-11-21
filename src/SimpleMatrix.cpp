#include "SimpleMatrix.h"

std::vector<SimpleMatrix*> SimpleMatrix::_instances;

SimpleMatrix::SimpleMatrix(const std::vector<uint8_t>& rowPins, const std::vector<uint8_t>& colPins, const std::vector<std::vector<char>>& layout)
    : _rowPins(rowPins),
      _colPins(colPins),
      _configured(false),
      _dummyButton([]() { return false; }),
      _lastPressedKey('\0') {
    for (size_t r = 0; r < layout.size() && r < _rowPins.size(); ++r) {
        for (size_t c = 0; c < layout[r].size() && c < _colPins.size(); ++c) {
            char ch = layout[r][c];
            if (ch != '\0') {
                KeyEntry entry;
                entry.keyChar = ch;
                entry.row = static_cast<uint8_t>(r);
                entry.col = static_cast<uint8_t>(c);
                entry.isDown = false;
                entry.button = nullptr;
                _keys.push_back(entry);
            }
        }
    }

    for (size_t i = 0; i < _keys.size(); ++i) {
        KeyEntry* entryPtr = &_keys[i];
        entryPtr->button = new SimpleButton([entryPtr]() {
            return entryPtr->isDown;
        });
    }

    _instances.push_back(this);
}

SimpleMatrix::~SimpleMatrix() {
    for (size_t i = 0; i < _instances.size(); ++i) {
        if (_instances[i] == this) {
            _instances.erase(_instances.begin() + i);
            break;
        }
    }
    for (auto& entry : _keys) {
        if (entry.button) {
            delete entry.button;
            entry.button = nullptr;
        }
    }
}

void SimpleMatrix::tickAll() {
    for (SimpleMatrix* instance : _instances) {
        if (instance) {
            instance->tick();
        }
    }
}

void SimpleMatrix::onKeyPress(KeyCallback callback) {
    _onKeyPress = callback;
}

void SimpleMatrix::onKeyRelease(KeyCallback callback) {
    _onKeyRelease = callback;
}

SimpleButton& SimpleMatrix::key(char keyChar) {
    for (auto& entry : _keys) {
        if (entry.keyChar == keyChar && entry.button) {
            return *(entry.button);
        }
    }
    return _dummyButton;
}

bool SimpleMatrix::isPressed(char keyChar) const {
    for (const auto& entry : _keys) {
        if (entry.keyChar == keyChar) {
            return entry.isDown;
        }
    }
    return false;
}

char SimpleMatrix::readKey() {
    char ch = _lastPressedKey;
    _lastPressedKey = '\0';
    return ch;
}

void SimpleMatrix::_scan() {
    if (!_configured) {
        for (uint8_t rPin : _rowPins) {
            pinMode(rPin, INPUT_PULLUP);
        }
        for (uint8_t cPin : _colPins) {
            pinMode(cPin, INPUT_PULLUP);
        }
        _configured = true;
    }

    for (size_t r = 0; r < _rowPins.size(); ++r) {
        pinMode(_rowPins[r], OUTPUT);
        digitalWrite(_rowPins[r], LOW);

        for (auto& entry : _keys) {
            if (entry.row == r) {
                bool pinLow = (digitalRead(_colPins[entry.col]) == LOW);
                if (pinLow != entry.isDown) {
                    entry.isDown = pinLow;
                    if (entry.isDown) {
                        _lastPressedKey = entry.keyChar;
                        if (_onKeyPress) {
                            _onKeyPress(entry.keyChar);
                        }
                    } else {
                        if (_onKeyRelease) {
                            _onKeyRelease(entry.keyChar);
                        }
                    }
                }
            }
        }

        digitalWrite(_rowPins[r], HIGH);
        pinMode(_rowPins[r], INPUT_PULLUP);
    }
}

void SimpleMatrix::tick() {
    _scan();
    for (auto& entry : _keys) {
        if (entry.button) {
            entry.button->tick();
        }
    }
}
