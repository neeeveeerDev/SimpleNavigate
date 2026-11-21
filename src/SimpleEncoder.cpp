#include "SimpleEncoder.h"

std::vector<SimpleEncoder*> SimpleEncoder::_instances;

SimpleEncoder::SimpleEncoder(uint8_t pinA, uint8_t pinB, int8_t pinBtn, uint8_t activeLevel, uint8_t pinConfig)
    : _pinA(pinA),
      _pinB(pinB),
      _pinConfig(pinConfig),
      _configured(false),
      _button(nullptr),
      _dummyButton([]() { return false; }),
      _lastState(0),
      _stepAccumulator(0),
      _stepsPerDetent(4),
      _inverted(false),
      _value(0),
      _minValue(0),
      _maxValue(0),
      _hasRange(false),
      _cyclic(false),
      _step(1),
      _accelEnabled(false),
      _fastThresholdMs(50),
      _accelMultiplier(5),
      _lastTurnTime(0),
      _isFast(false),
      _flagLeft(false),
      _flagRight(false),
      _deltaAccumulator(0) {
    if (pinBtn >= 0) {
        _button = new SimpleButton(static_cast<uint8_t>(pinBtn), activeLevel, pinConfig);
    }
    _instances.push_back(this);
}

SimpleEncoder::~SimpleEncoder() {
    for (size_t i = 0; i < _instances.size(); ++i) {
        if (_instances[i] == this) {
            _instances.erase(_instances.begin() + i);
            break;
        }
    }
    if (_button) {
        delete _button;
        _button = nullptr;
    }
}

void SimpleEncoder::tickAll() {
    for (SimpleEncoder* instance : _instances) {
        if (instance) {
            instance->tick();
        }
    }
}

void SimpleEncoder::onLeft(Callback callback) {
    _onLeft = callback;
}

void SimpleEncoder::onRight(Callback callback) {
    _onRight = callback;
}

void SimpleEncoder::onLeftHold(Callback callback) {
    _onLeftHold = callback;
}

void SimpleEncoder::onRightHold(Callback callback) {
    _onRightHold = callback;
}

void SimpleEncoder::onChange(ChangeCallback callback) {
    _onChange = callback;
}

int16_t SimpleEncoder::getValue() const {
    return _value;
}

void SimpleEncoder::setValue(int16_t val) {
    _value = val;
    if (_hasRange) {
        if (_value < _minValue) _value = _minValue;
        if (_value > _maxValue) _value = _maxValue;
    }
}

int16_t SimpleEncoder::getDelta() {
    int16_t d = _deltaAccumulator;
    _deltaAccumulator = 0;
    return d;
}

bool SimpleEncoder::hasTurnedLeft() {
    bool state = _flagLeft;
    _flagLeft = false;
    return state;
}

bool SimpleEncoder::hasTurnedRight() {
    bool state = _flagRight;
    _flagRight = false;
    return state;
}

bool SimpleEncoder::isFast() const {
    return _isFast;
}

void SimpleEncoder::setRange(int16_t minVal, int16_t maxVal, bool cyclic) {
    _hasRange = true;
    _minValue = minVal;
    _maxValue = maxVal;
    _cyclic = cyclic;
    if (_value < _minValue) _value = _minValue;
    if (_value > _maxValue) _value = _maxValue;
}

void SimpleEncoder::clearRange() {
    _hasRange = false;
}

void SimpleEncoder::setStep(int16_t step) {
    _step = step;
}

void SimpleEncoder::setStepsPerDetent(uint8_t steps) {
    _stepsPerDetent = (steps == 0) ? 1 : steps;
}

void SimpleEncoder::setInverted(bool inverted) {
    _inverted = inverted;
}

void SimpleEncoder::setAcceleration(bool enabled, uint16_t fastThresholdMs, uint8_t multiplier) {
    _accelEnabled = enabled;
    _fastThresholdMs = fastThresholdMs;
    _accelMultiplier = (multiplier == 0) ? 1 : multiplier;
}

SimpleButton& SimpleEncoder::btn() {
    if (_button) {
        return *_button;
    }
    return _dummyButton;
}

bool SimpleEncoder::hasButton() const {
    return _button != nullptr;
}

void SimpleEncoder::tick() {
    if (!_configured) {
        pinMode(_pinA, _pinConfig);
        pinMode(_pinB, _pinConfig);
        _lastState = (digitalRead(_pinA) << 1) | digitalRead(_pinB);
        _configured = true;
    }

    if (_button) {
        _button->tick();
    }

    uint8_t s = (digitalRead(_pinA) << 1) | digitalRead(_pinB);
    if (s != _lastState) {
        static const int8_t table[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
        uint8_t index = static_cast<uint8_t>((_lastState << 2) | s);
        int8_t movement = table[index & 0x0F];
        _lastState = s;

        if (movement != 0) {
            _stepAccumulator += movement;

            if (abs(_stepAccumulator) >= _stepsPerDetent) {
                int8_t direction = (_stepAccumulator > 0 ? 1 : -1) * (_inverted ? -1 : 1);
                _stepAccumulator = 0;

                uint32_t now = millis();
                uint32_t dt = now - _lastTurnTime;
                _lastTurnTime = now;

                _isFast = (dt < _fastThresholdMs);
                int16_t currentMultiplier = (_accelEnabled && _isFast) ? _accelMultiplier : 1;
                int16_t delta = direction * _step * currentMultiplier;

                if (_hasRange) {
                    if (_cyclic) {
                        int32_t span = static_cast<int32_t>(_maxValue) - static_cast<int32_t>(_minValue) + 1;
                        int32_t nextVal = static_cast<int32_t>(_value) + delta;
                        while (nextVal < _minValue) nextVal += span;
                        while (nextVal > _maxValue) nextVal -= span;
                        _value = static_cast<int16_t>(nextVal);
                    } else {
                        int32_t nextVal = static_cast<int32_t>(_value) + delta;
                        if (nextVal < _minValue) nextVal = _minValue;
                        if (nextVal > _maxValue) nextVal = _maxValue;
                        _value = static_cast<int16_t>(nextVal);
                    }
                } else {
                    _value += delta;
                }

                _deltaAccumulator += delta;

                bool btnPressed = (_button && _button->isPressed());
                if (direction > 0) {
                    _flagRight = true;
                    if (btnPressed && _onRightHold) {
                        _onRightHold();
                    } else if (_onRight) {
                        _onRight();
                    }
                } else {
                    _flagLeft = true;
                    if (btnPressed && _onLeftHold) {
                        _onLeftHold();
                    } else if (_onLeft) {
                        _onLeft();
                    }
                }

                if (_onChange) {
                    _onChange(delta, _value);
                }
            }
        }
    } else {
        if (millis() - _lastTurnTime >= _fastThresholdMs * 2) {
            _isFast = false;
        }
    }
}
