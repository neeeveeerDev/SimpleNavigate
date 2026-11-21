#include "SimpleJoystick.h"
#include <math.h>

std::vector<SimpleJoystick*> SimpleJoystick::_instances;

SimpleJoystick::SimpleJoystick(uint8_t pinX, uint8_t pinY, int8_t pinBtn, uint8_t activeLevel)
    : _pinX(pinX),
      _pinY(pinY),
      _btnStick(nullptr),
      _dummyButton([]() { return false; }),
      _btnUp([this]() { return _currentDir == Direction::Up; }),
      _btnDown([this]() { return _currentDir == Direction::Down; }),
      _btnLeft([this]() { return _currentDir == Direction::Left; }),
      _btnRight([this]() { return _currentDir == Direction::Right; }),
#if defined(ESP32)
      _maxAdc(4095),
      _centerX(2048.0f),
      _centerY(2048.0f),
#else
      _maxAdc(1023),
      _centerX(512.0f),
      _centerY(512.0f),
#endif
      _calibrated(false),
      _invertX(false),
      _invertY(false),
      _swapAxes(false),
      _deadzone(10),
      _outerDeadzone(5),
      _smoothing(0.25f),
      _exponent(1.0f),
      _filteredX(_centerX),
      _filteredY(_centerY),
      _normX(0),
      _normY(0),
      _magnitude(0),
      _angle(0),
      _currentDir(Direction::Center),
      _lastDir(Direction::Center),
      _currentDir8(Direction8::Center),
      _lastDir8(Direction8::Center),
      _dirHoldTimer(0),
      _dirStepTimer(0),
      _repeatHoldMs(500),
      _repeatStepMs(150),
      _lastFlickDir(Direction::Center),
      _flickStartTime(0),
      _lastFlickCompleteTime(0),
      _flickCount(0) {
    if (pinBtn >= 0) {
        _btnStick = new SimpleButton(static_cast<uint8_t>(pinBtn), activeLevel, INPUT_PULLUP);
    }
    _instances.push_back(this);
}

SimpleJoystick::~SimpleJoystick() {
    for (size_t i = 0; i < _instances.size(); ++i) {
        if (_instances[i] == this) {
            _instances.erase(_instances.begin() + i);
            break;
        }
    }
    if (_btnStick) {
        delete _btnStick;
        _btnStick = nullptr;
    }
}

void SimpleJoystick::tickAll() {
    for (SimpleJoystick* instance : _instances) {
        if (instance) {
            instance->tick();
        }
    }
}

int8_t SimpleJoystick::getX() const {
    return _normX;
}

int8_t SimpleJoystick::getY() const {
    return _normY;
}

uint16_t SimpleJoystick::getRawX() const {
    return static_cast<uint16_t>(_filteredX);
}

uint16_t SimpleJoystick::getRawY() const {
    return static_cast<uint16_t>(_filteredY);
}

uint8_t SimpleJoystick::getMagnitude() const {
    return _magnitude;
}

uint16_t SimpleJoystick::getAngle() const {
    return _angle;
}

SimpleJoystick::Direction SimpleJoystick::getDirection() const {
    return _currentDir;
}

SimpleJoystick::Direction8 SimpleJoystick::getDirection8() const {
    return _currentDir8;
}

void SimpleJoystick::setInvertX(bool invert) {
    _invertX = invert;
}

void SimpleJoystick::setInvertY(bool invert) {
    _invertY = invert;
}

void SimpleJoystick::setSwapAxes(bool swap) {
    _swapAxes = swap;
}

void SimpleJoystick::setDeadzone(uint8_t percent) {
    _deadzone = percent > 50 ? 50 : percent;
}

void SimpleJoystick::setOuterDeadzone(uint8_t percent) {
    _outerDeadzone = percent > 30 ? 30 : percent;
}

void SimpleJoystick::setSmoothing(float factor) {
    if (factor < 0.01f) factor = 0.01f;
    if (factor > 1.0f) factor = 1.0f;
    _smoothing = factor;
}

void SimpleJoystick::setExponent(float expo) {
    if (expo < 1.0f) expo = 1.0f;
    if (expo > 3.0f) expo = 3.0f;
    _exponent = expo;
}

void SimpleJoystick::setResolution(uint16_t maxAdc) {
    _maxAdc = maxAdc;
    if (!_calibrated) {
        _centerX = maxAdc / 2.0f;
        _centerY = maxAdc / 2.0f;
    }
}

void SimpleJoystick::calibrateCenter() {
    uint32_t sumX = 0;
    uint32_t sumY = 0;
    for (uint8_t i = 0; i < 16; ++i) {
        sumX += analogRead(_pinX);
        sumY += analogRead(_pinY);
        delay(2);
    }
    _centerX = sumX / 16.0f;
    _centerY = sumY / 16.0f;
    _filteredX = _centerX;
    _filteredY = _centerY;
    _calibrated = true;
}

void SimpleJoystick::setRepeat(uint16_t holdMs, uint16_t stepMs) {
    _repeatHoldMs = holdMs;
    _repeatStepMs = stepMs;
}

void SimpleJoystick::onUp(Callback callback) {
    _onUp = callback;
}

void SimpleJoystick::onDown(Callback callback) {
    _onDown = callback;
}

void SimpleJoystick::onLeft(Callback callback) {
    _onLeft = callback;
}

void SimpleJoystick::onRight(Callback callback) {
    _onRight = callback;
}

void SimpleJoystick::onUpLeft(Callback callback) {
    _onUpLeft = callback;
}

void SimpleJoystick::onUpRight(Callback callback) {
    _onUpRight = callback;
}

void SimpleJoystick::onDownLeft(Callback callback) {
    _onDownLeft = callback;
}

void SimpleJoystick::onDownRight(Callback callback) {
    _onDownRight = callback;
}

void SimpleJoystick::onFlick(Direction dir, Callback callback) {
    _flickCallbacks.push_back({dir, callback});
}

void SimpleJoystick::onDoubleFlick(Direction dir, Callback callback) {
    _doubleFlickCallbacks.push_back({dir, callback});
}

SimpleButton& SimpleJoystick::btn() {
    if (_btnStick) {
        return *_btnStick;
    }
    return _dummyButton;
}

SimpleButton& SimpleJoystick::btnUp() {
    return _btnUp;
}

SimpleButton& SimpleJoystick::btnDown() {
    return _btnDown;
}

SimpleButton& SimpleJoystick::btnLeft() {
    return _btnLeft;
}

SimpleButton& SimpleJoystick::btnRight() {
    return _btnRight;
}

int8_t SimpleJoystick::_calculateNormalized(float raw, float center, bool invert) {
    float delta = raw - center;
    float maxDelta = (delta >= 0) ? (_maxAdc - center) : center;
    if (maxDelta <= 0.001f) {
        return 0;
    }

    float ratio = (delta / maxDelta) * 100.0f;
    if (invert) {
        ratio = -ratio;
    }

    float absRatio = fabs(ratio);
    if (absRatio <= _deadzone) {
        return 0;
    }

    float effectiveMax = 100.0f - _outerDeadzone;
    float scaled = (absRatio - _deadzone) / (effectiveMax - _deadzone) * 100.0f;
    if (scaled > 100.0f) {
        scaled = 100.0f;
    }

    if (_exponent > 1.001f) {
        scaled = powf(scaled / 100.0f, _exponent) * 100.0f;
    }

    int8_t result = static_cast<int8_t>(roundf(scaled));
    return (ratio < 0) ? -result : result;
}

void SimpleJoystick::tick() {
    if (!_calibrated) {
        calibrateCenter();
    }

    if (_btnStick) {
        _btnStick->tick();
    }

    uint16_t rawX = analogRead(_pinX);
    uint16_t rawY = analogRead(_pinY);

    _filteredX = (_filteredX * (1.0f - _smoothing)) + (rawX * _smoothing);
    _filteredY = (_filteredY * (1.0f - _smoothing)) + (rawY * _smoothing);

    float axisX = _swapAxes ? _filteredY : _filteredX;
    float axisY = _swapAxes ? _filteredX : _filteredY;
    float centerAxisX = _swapAxes ? _centerY : _centerX;
    float centerAxisY = _swapAxes ? _centerX : _centerY;

    _normX = _calculateNormalized(axisX, centerAxisX, _invertX);
    _normY = _calculateNormalized(axisY, centerAxisY, _invertY);

    float fx = static_cast<float>(_normX);
    float fy = static_cast<float>(_normY);
    float mag = sqrtf(fx * fx + fy * fy);
    _magnitude = (mag > 100.0f) ? 100 : static_cast<uint8_t>(roundf(mag));

    if (_magnitude > 0) {
        float rad = atan2f(fy, fx);
        float deg = rad * 180.0f / 3.14159265f;
        if (deg < 0.0f) {
            deg += 360.0f;
        }
        _angle = static_cast<uint16_t>(roundf(deg));
    } else {
        _angle = 0;
    }

    Direction nextDir = Direction::Center;
    if (_magnitude >= 40) {
        if (_angle >= 45 && _angle < 135) {
            nextDir = Direction::Up;
        } else if (_angle >= 135 && _angle < 225) {
            nextDir = Direction::Left;
        } else if (_angle >= 225 && _angle < 315) {
            nextDir = Direction::Down;
        } else {
            nextDir = Direction::Right;
        }
    }

    Direction8 nextDir8 = Direction8::Center;
    if (_magnitude >= 40) {
        if (_angle >= 337 || _angle < 23) {
            nextDir8 = Direction8::Right;
        } else if (_angle >= 23 && _angle < 68) {
            nextDir8 = Direction8::UpRight;
        } else if (_angle >= 68 && _angle < 113) {
            nextDir8 = Direction8::Up;
        } else if (_angle >= 113 && _angle < 158) {
            nextDir8 = Direction8::UpLeft;
        } else if (_angle >= 158 && _angle < 203) {
            nextDir8 = Direction8::Left;
        } else if (_angle >= 203 && _angle < 248) {
            nextDir8 = Direction8::DownLeft;
        } else if (_angle >= 248 && _angle < 293) {
            nextDir8 = Direction8::Down;
        } else {
            nextDir8 = Direction8::DownRight;
        }
    }

    uint32_t now = millis();

    if (nextDir != _currentDir) {
        _lastDir = _currentDir;
        _currentDir = nextDir;

        if (_currentDir != Direction::Center) {
            _dirHoldTimer = now;
            _dirStepTimer = now;

            if (_currentDir == Direction::Up && _onUp) _onUp();
            else if (_currentDir == Direction::Down && _onDown) _onDown();
            else if (_currentDir == Direction::Left && _onLeft) _onLeft();
            else if (_currentDir == Direction::Right && _onRight) _onRight();

            _lastFlickDir = _currentDir;
            _flickStartTime = now;
        } else {
            if (_lastFlickDir != Direction::Center && (now - _flickStartTime <= 220)) {
                if (_flickCount > 0 && (now - _lastFlickCompleteTime <= 380)) {
                    for (const auto& item : _doubleFlickCallbacks) {
                        if (item.first == _lastFlickDir && item.second) {
                            item.second();
                        }
                    }
                    _flickCount = 0;
                } else {
                    for (const auto& item : _flickCallbacks) {
                        if (item.first == _lastFlickDir && item.second) {
                            item.second();
                        }
                    }
                    _flickCount = 1;
                    _lastFlickCompleteTime = now;
                }
            }
            _lastFlickDir = Direction::Center;
        }
    } else if (_currentDir != Direction::Center) {
        if (now - _dirHoldTimer >= _repeatHoldMs) {
            if (now - _dirStepTimer >= _repeatStepMs) {
                _dirStepTimer = now;
                if (_currentDir == Direction::Up && _onUp) _onUp();
                else if (_currentDir == Direction::Down && _onDown) _onDown();
                else if (_currentDir == Direction::Left && _onLeft) _onLeft();
                else if (_currentDir == Direction::Right && _onRight) _onRight();
            }
        }
    }

    if (nextDir8 != _currentDir8) {
        _lastDir8 = _currentDir8;
        _currentDir8 = nextDir8;
        if (_currentDir8 == Direction8::UpLeft && _onUpLeft) _onUpLeft();
        else if (_currentDir8 == Direction8::UpRight && _onUpRight) _onUpRight();
        else if (_currentDir8 == Direction8::DownLeft && _onDownLeft) _onDownLeft();
        else if (_currentDir8 == Direction8::DownRight && _onDownRight) _onDownRight();
    }

    _btnUp.tick();
    _btnDown.tick();
    _btnLeft.tick();
    _btnRight.tick();
}
