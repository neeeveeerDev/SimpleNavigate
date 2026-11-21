#include "SimpleAnalogKeypad.h"
#include <math.h>

std::vector<SimpleAnalogKeypad*> SimpleAnalogKeypad::_instances;

SimpleAnalogKeypad::SimpleAnalogKeypad(uint8_t analogPin, uint16_t tolerance)
    : _pin(analogPin),
      _tolerance(tolerance),
      _smoothing(0.3f),
      _filteredAdc(0.0f),
      _dummyButton([]() { return false; }),
      _activeKey('\0'),
      _lastPressedKey('\0') {
    _instances.push_back(this);
}

SimpleAnalogKeypad::~SimpleAnalogKeypad() {
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

void SimpleAnalogKeypad::tickAll() {
    for (SimpleAnalogKeypad* instance : _instances) {
        if (instance) {
            instance->tick();
        }
    }
}

void SimpleAnalogKeypad::addKey(char id, uint16_t expectedAdc) {
    for (auto& entry : _keys) {
        if (entry.id == id) {
            entry.expectedAdc = expectedAdc;
            return;
        }
    }

    AnalogKeyEntry entry;
    entry.id = id;
    entry.expectedAdc = expectedAdc;
    entry.isDown = false;
    entry.button = nullptr;
    _keys.push_back(entry);

    AnalogKeyEntry* entryPtr = &_keys.back();
    entryPtr->button = new SimpleButton([entryPtr]() {
        return entryPtr->isDown;
    });
}

void SimpleAnalogKeypad::setTolerance(uint16_t tolerance) {
    _tolerance = tolerance;
}

void SimpleAnalogKeypad::setSmoothing(float factor) {
    if (factor < 0.01f) factor = 0.01f;
    if (factor > 1.0f) factor = 1.0f;
    _smoothing = factor;
}

SimpleButton& SimpleAnalogKeypad::key(char id) {
    for (auto& entry : _keys) {
        if (entry.id == id && entry.button) {
            return *(entry.button);
        }
    }
    return _dummyButton;
}

bool SimpleAnalogKeypad::isPressed(char id) const {
    for (const auto& entry : _keys) {
        if (entry.id == id) {
            return entry.isDown;
        }
    }
    return false;
}

char SimpleAnalogKeypad::readKey() {
    char ch = _lastPressedKey;
    _lastPressedKey = '\0';
    return ch;
}

uint16_t SimpleAnalogKeypad::getRawAdc() const {
    return static_cast<uint16_t>(roundf(_filteredAdc));
}

void SimpleAnalogKeypad::onKeyPress(KeyCallback callback) {
    _onKeyPress = callback;
}

void SimpleAnalogKeypad::onKeyRelease(KeyCallback callback) {
    _onKeyRelease = callback;
}

void SimpleAnalogKeypad::tick() {
    uint16_t raw = analogRead(_pin);
    if (_filteredAdc <= 0.001f) {
        _filteredAdc = raw;
    } else {
        _filteredAdc = (_filteredAdc * (1.0f - _smoothing)) + (raw * _smoothing);
    }

    uint16_t currentAdc = static_cast<uint16_t>(roundf(_filteredAdc));

    char detectedKey = '\0';
    uint16_t bestDiff = 0xFFFF;

    for (const auto& entry : _keys) {
        uint16_t diff = (currentAdc >= entry.expectedAdc) ? (currentAdc - entry.expectedAdc) : (entry.expectedAdc - currentAdc);
        if (diff <= _tolerance && diff < bestDiff) {
            bestDiff = diff;
            detectedKey = entry.id;
        }
    }

    if (detectedKey != _activeKey) {
        char oldKey = _activeKey;
        _activeKey = detectedKey;

        for (auto& entry : _keys) {
            bool nextState = (entry.id == _activeKey);
            if (nextState != entry.isDown) {
                entry.isDown = nextState;
            }
        }

        if (oldKey != '\0' && _onKeyRelease) {
            _onKeyRelease(oldKey);
        }

        if (_activeKey != '\0') {
            _lastPressedKey = _activeKey;
            if (_onKeyPress) {
                _onKeyPress(_activeKey);
            }
        }
    }

    for (auto& entry : _keys) {
        if (entry.button) {
            entry.button->tick();
        }
    }
}
