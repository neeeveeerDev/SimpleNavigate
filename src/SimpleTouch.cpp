#include "SimpleTouch.h"

std::vector<SimpleTouch*> SimpleTouch::_instances;

SimpleTouch::SimpleTouch(uint8_t touchPin, uint16_t thresholdDelta)
    : _pin(touchPin),
      _thresholdDelta(thresholdDelta),
      _baseline(0),
      _rawValue(0),
      _calibrated(false),
      _isTouched(false),
      _button([this]() { return _isTouched; }) {
    _instances.push_back(this);
}

SimpleTouch::~SimpleTouch() {
    for (size_t i = 0; i < _instances.size(); ++i) {
        if (_instances[i] == this) {
            _instances.erase(_instances.begin() + i);
            break;
        }
    }
}

void SimpleTouch::tickAll() {
    for (SimpleTouch* instance : _instances) {
        if (instance) {
            instance->tick();
        }
    }
}

void SimpleTouch::calibrate() {
#if defined(ESP32)
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 16; ++i) {
        sum += touchRead(_pin);
        delay(2);
    }
    _baseline = static_cast<uint16_t>(sum / 16);
#else
    _baseline = 100;
#endif
    _calibrated = true;
}

SimpleButton& SimpleTouch::btn() {
    return _button;
}

bool SimpleTouch::isTouched() const {
    return _isTouched;
}

uint16_t SimpleTouch::getRawValue() const {
    return _rawValue;
}

uint16_t SimpleTouch::getBaseline() const {
    return _baseline;
}

void SimpleTouch::setThresholdDelta(uint16_t delta) {
    _thresholdDelta = delta;
}

void SimpleTouch::tick() {
    if (!_calibrated) {
        calibrate();
    }

#if defined(ESP32)
    _rawValue = static_cast<uint16_t>(touchRead(_pin));
    if (_baseline > _thresholdDelta) {
        _isTouched = (_rawValue < (_baseline - _thresholdDelta));
    } else {
        _isTouched = false;
    }
#else
    _rawValue = 0;
    _isTouched = false;
#endif

    _button.tick();
}
