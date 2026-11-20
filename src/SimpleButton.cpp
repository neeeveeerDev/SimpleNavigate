#include "SimpleButton.h"

std::vector<SimpleButton*> SimpleButton::_instances;
std::vector<SimpleButtonCombo*> SimpleButtonCombo::_comboInstances;

#if defined(ESP32)
TaskHandle_t SimpleButton::_pollTaskHandle = nullptr;

void SimpleButton::_backgroundPollTask(void* parameter) {
    uint32_t interval = (parameter != nullptr) ? *reinterpret_cast<uint32_t*>(parameter) : 5;
    delete reinterpret_cast<uint32_t*>(parameter);

    TickType_t delayTicks = pdMS_TO_TICKS(interval);
    if (delayTicks == 0) {
        delayTicks = 1;
    }

    while (true) {
        SimpleButton::tickAll();
        vTaskDelay(delayTicks);
    }
}

void SimpleButton::startBackgroundPoll(uint32_t intervalMs, uint8_t core) {
    if (_pollTaskHandle != nullptr) {
        return;
    }
    uint32_t* intervalParam = new uint32_t(intervalMs);
    xTaskCreatePinnedToCore(
        _backgroundPollTask,
        "SimpleButtonTask",
        4096,
        intervalParam,
        1,
        &_pollTaskHandle,
        core
    );
}

void SimpleButton::stopBackgroundPoll() {
    if (_pollTaskHandle != nullptr) {
        vTaskDelete(_pollTaskHandle);
        _pollTaskHandle = nullptr;
    }
}

void SimpleButton::enableWakeup() {
    if (_pin <= 39 && _reader == nullptr) {
        esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(_pin), _activeLevel == LOW ? 0 : 1);
    }
}
#endif

SimpleButton::SimpleButton(uint8_t pin, uint8_t activeLevel, uint8_t pinConfig)
    : _pin(pin),
      _activeLevel(activeLevel),
      _pinConfig(pinConfig),
      _configured(false),
      _reader(nullptr),
      _state(State::Idle),
      _stateTimer(0),
      _pressStartTime(0),
      _lastStepTime(0),
      _lastHoldTime(0),
      _clickCount(0),
      _lastReturnedClicks(0),
      _debouncedState(false),
      _flagPress(false),
      _flagRelease(false),
      _flagClick(false),
      _flagDouble(false),
      _flagHold(false),
      _flagStep(false),
      _toggleState(false),
      _debounceMs(40),
      _holdTimeMs(600),
      _stepTimeMs(150),
      _clickTimeoutMs(300),
      _minStepIntervalMs(0),
      _stepDecayMs(0),
      _currentStepIntervalMs(150),
      _stuckTimeoutMs(0) {
    _instances.push_back(this);
}

SimpleButton::SimpleButton(CustomReader reader)
    : _pin(255),
      _activeLevel(HIGH),
      _pinConfig(0),
      _configured(true),
      _reader(reader),
      _state(State::Idle),
      _stateTimer(0),
      _pressStartTime(0),
      _lastStepTime(0),
      _lastHoldTime(0),
      _clickCount(0),
      _lastReturnedClicks(0),
      _debouncedState(false),
      _flagPress(false),
      _flagRelease(false),
      _flagClick(false),
      _flagDouble(false),
      _flagHold(false),
      _flagStep(false),
      _toggleState(false),
      _debounceMs(40),
      _holdTimeMs(600),
      _stepTimeMs(150),
      _clickTimeoutMs(300),
      _minStepIntervalMs(0),
      _stepDecayMs(0),
      _currentStepIntervalMs(150),
      _stuckTimeoutMs(0) {
    _instances.push_back(this);
}

SimpleButton::~SimpleButton() {
    for (size_t i = 0; i < _instances.size(); ++i) {
        if (_instances[i] == this) {
            _instances.erase(_instances.begin() + i);
            break;
        }
    }
}

void SimpleButton::tickAll() {
    for (SimpleButton* instance : _instances) {
        if (instance) {
            instance->tick();
        }
    }
    SimpleButtonCombo::tickAllCombos();
}

void SimpleButton::onPress(Callback callback) {
    _onPress = callback;
}

void SimpleButton::onRelease(Callback callback) {
    _onRelease = callback;
}

void SimpleButton::onClick(Callback callback) {
    _onClick = callback;
}

void SimpleButton::onDoubleClick(Callback callback) {
    _onDoubleClick = callback;
}

void SimpleButton::onMultiClick(MultiCallback callback) {
    _onMultiClick = callback;
}

void SimpleButton::onHold(Callback callback) {
    _onHold = callback;
}

void SimpleButton::onHoldFor(uint16_t durationMs, Callback callback) {
    _holdThresholds.push_back({durationMs, callback, false});
}

void SimpleButton::onStep(Callback callback) {
    _onStep = callback;
}

void SimpleButton::onToggle(ToggleCallback callback) {
    _onToggle = callback;
}

void SimpleButton::onStuck(Callback callback) {
    _onStuck = callback;
}

bool SimpleButton::isPressed() const {
    return _debouncedState;
}

bool SimpleButton::hasPressed() {
    bool state = _flagPress;
    _flagPress = false;
    return state;
}

bool SimpleButton::hasReleased() {
    bool state = _flagRelease;
    _flagRelease = false;
    return state;
}

bool SimpleButton::isClick() {
    bool state = _flagClick;
    _flagClick = false;
    return state;
}

bool SimpleButton::isDouble() {
    bool state = _flagDouble;
    _flagDouble = false;
    return state;
}

bool SimpleButton::isHold() {
    bool state = _flagHold;
    _flagHold = false;
    return state;
}

bool SimpleButton::isStep() {
    bool state = _flagStep;
    _flagStep = false;
    return state;
}

bool SimpleButton::isToggled() const {
    return _toggleState;
}

bool SimpleButton::isStuck() const {
    return _state == State::Stuck;
}

uint8_t SimpleButton::getClicks() {
    uint8_t count = _lastReturnedClicks;
    _lastReturnedClicks = 0;
    return count;
}

uint32_t SimpleButton::getHoldTime() const {
    if (_debouncedState) {
        return millis() - _pressStartTime;
    }
    return _lastHoldTime;
}

void SimpleButton::setToggle(bool state) {
    _toggleState = state;
}

void SimpleButton::setDebounce(uint16_t ms) {
    _debounceMs = ms;
}

void SimpleButton::setHoldTime(uint16_t ms) {
    _holdTimeMs = ms;
}

void SimpleButton::setStepTime(uint16_t ms) {
    _stepTimeMs = ms;
}

void SimpleButton::setClickTimeout(uint16_t ms) {
    _clickTimeoutMs = ms;
}

void SimpleButton::setStepAcceleration(uint16_t minIntervalMs, uint16_t stepDecayMs) {
    _minStepIntervalMs = minIntervalMs;
    _stepDecayMs = stepDecayMs;
}

void SimpleButton::setStuckTimeout(uint32_t ms) {
    _stuckTimeoutMs = ms;
}

void SimpleButton::reset() {
    _state = State::Idle;
    _stateTimer = 0;
    _pressStartTime = 0;
    _lastStepTime = 0;
    _lastHoldTime = 0;
    _clickCount = 0;
    _lastReturnedClicks = 0;
    _debouncedState = false;
    _flagPress = false;
    _flagRelease = false;
    _flagClick = false;
    _flagDouble = false;
    _flagHold = false;
    _flagStep = false;
    for (auto& threshold : _holdThresholds) {
        threshold.triggered = false;
    }
}

bool SimpleButton::_readRaw() {
    if (_reader) {
        return _reader();
    }
    if (!_configured) {
        pinMode(_pin, _pinConfig);
        _configured = true;
    }
    return digitalRead(_pin) == _activeLevel;
}

void SimpleButton::tick() {
    _flagPress = false;
    _flagRelease = false;
    _flagClick = false;
    _flagDouble = false;
    _flagHold = false;
    _flagStep = false;

    uint32_t now = millis();
    bool raw = _readRaw();

    switch (_state) {
        case State::Idle: {
            if (raw) {
                _state = State::DebouncePress;
                _stateTimer = now;
            }
            break;
        }

        case State::DebouncePress: {
            if (!raw) {
                _state = State::Idle;
            } else if (now - _stateTimer >= _debounceMs) {
                _state = State::Pressed;
                _debouncedState = true;
                _flagPress = true;
                _pressStartTime = now;
                _currentStepIntervalMs = _stepTimeMs;
                for (auto& threshold : _holdThresholds) {
                    threshold.triggered = false;
                }
                if (_onPress) {
                    _onPress();
                }
            }
            break;
        }

        case State::Pressed: {
            if (!raw) {
                _state = State::DebounceRelease;
                _stateTimer = now;
            } else {
                uint32_t heldDuration = now - _pressStartTime;

                if (_stuckTimeoutMs > 0 && heldDuration >= _stuckTimeoutMs) {
                    _state = State::Stuck;
                    if (_onStuck) {
                        _onStuck();
                    }
                    break;
                }

                for (auto& threshold : _holdThresholds) {
                    if (!threshold.triggered && heldDuration >= threshold.duration) {
                        threshold.triggered = true;
                        if (threshold.callback) {
                            threshold.callback();
                        }
                    }
                }

                if (heldDuration >= _holdTimeMs) {
                    _state = State::Holding;
                    _lastStepTime = now;
                    _flagHold = true;
                    if (_onHold) {
                        _onHold();
                    }
                    _flagStep = true;
                    if (_onStep) {
                        _onStep();
                    }
                }
            }
            break;
        }

        case State::Holding: {
            if (!raw) {
                _state = State::DebounceRelease;
                _stateTimer = now;
            } else {
                uint32_t heldDuration = now - _pressStartTime;

                if (_stuckTimeoutMs > 0 && heldDuration >= _stuckTimeoutMs) {
                    _state = State::Stuck;
                    if (_onStuck) {
                        _onStuck();
                    }
                    break;
                }

                for (auto& threshold : _holdThresholds) {
                    if (!threshold.triggered && heldDuration >= threshold.duration) {
                        threshold.triggered = true;
                        if (threshold.callback) {
                            threshold.callback();
                        }
                    }
                }

                if (now - _lastStepTime >= _currentStepIntervalMs) {
                    _lastStepTime = now;
                    _flagStep = true;
                    if (_onStep) {
                        _onStep();
                    }

                    if (_stepDecayMs > 0 && _currentStepIntervalMs > _minStepIntervalMs) {
                        if (_currentStepIntervalMs - _minStepIntervalMs > _stepDecayMs) {
                            _currentStepIntervalMs -= _stepDecayMs;
                        } else {
                            _currentStepIntervalMs = _minStepIntervalMs;
                        }
                    }
                }
            }
            break;
        }

        case State::Stuck: {
            if (!raw) {
                _state = State::DebounceRelease;
                _stateTimer = now;
            }
            break;
        }

        case State::DebounceRelease: {
            if (raw) {
                _state = (_debouncedState && (now - _pressStartTime >= _holdTimeMs)) ? State::Holding : State::Pressed;
            } else if (now - _stateTimer >= _debounceMs) {
                _debouncedState = false;
                _flagRelease = true;
                _lastHoldTime = now - _pressStartTime;
                if (_onRelease) {
                    _onRelease();
                }

                if (_lastHoldTime >= _holdTimeMs || (_stuckTimeoutMs > 0 && _lastHoldTime >= _stuckTimeoutMs)) {
                    _clickCount = 0;
                    _state = State::Idle;
                } else {
                    _clickCount++;
                    _state = State::WaitNextClick;
                    _stateTimer = now;
                }
            }
            break;
        }

        case State::WaitNextClick: {
            if (raw) {
                _state = State::DebouncePress;
                _stateTimer = now;
            } else if (now - _stateTimer >= _clickTimeoutMs) {
                _lastReturnedClicks = _clickCount;
                if (_clickCount == 1) {
                    _flagClick = true;
                    _toggleState = !_toggleState;
                    if (_onClick) {
                        _onClick();
                    }
                    if (_onToggle) {
                        _onToggle(_toggleState);
                    }
                } else if (_clickCount == 2) {
                    _flagDouble = true;
                    if (_onDoubleClick) {
                        _onDoubleClick();
                    }
                }
                if (_clickCount > 0 && _onMultiClick) {
                    _onMultiClick(_clickCount);
                }
                _clickCount = 0;
                _state = State::Idle;
            }
            break;
        }
    }
}

SimpleButtonCombo::SimpleButtonCombo(SimpleButton& first, SimpleButton& second)
    : _holdTimeMs(600),
      _pressStartTime(0),
      _isPressed(false),
      _isHolding(false),
      _flagClick(false),
      _flagHold(false) {
    _buttons.push_back(&first);
    _buttons.push_back(&second);
    _comboInstances.push_back(this);
}

SimpleButtonCombo::SimpleButtonCombo(std::vector<SimpleButton*> buttons)
    : _buttons(buttons),
      _holdTimeMs(600),
      _pressStartTime(0),
      _isPressed(false),
      _isHolding(false),
      _flagClick(false),
      _flagHold(false) {
    _comboInstances.push_back(this);
}

SimpleButtonCombo::~SimpleButtonCombo() {
    for (size_t i = 0; i < _comboInstances.size(); ++i) {
        if (_comboInstances[i] == this) {
            _comboInstances.erase(_comboInstances.begin() + i);
            break;
        }
    }
}

void SimpleButtonCombo::tickAllCombos() {
    for (SimpleButtonCombo* combo : _comboInstances) {
        if (combo) {
            combo->tick();
        }
    }
}

void SimpleButtonCombo::setHoldTime(uint16_t ms) {
    _holdTimeMs = ms;
}

void SimpleButtonCombo::onPress(Callback callback) {
    _onPress = callback;
}

void SimpleButtonCombo::onRelease(Callback callback) {
    _onRelease = callback;
}

void SimpleButtonCombo::onClick(Callback callback) {
    _onClick = callback;
}

void SimpleButtonCombo::onHold(Callback callback) {
    _onHold = callback;
}

bool SimpleButtonCombo::isPressed() const {
    return _isPressed;
}

bool SimpleButtonCombo::isClick() {
    bool state = _flagClick;
    _flagClick = false;
    return state;
}

bool SimpleButtonCombo::isHold() {
    bool state = _flagHold;
    _flagHold = false;
    return state;
}

void SimpleButtonCombo::tick() {
    _flagClick = false;
    _flagHold = false;

    if (_buttons.empty()) {
        return;
    }

    bool allPressed = true;
    for (SimpleButton* btn : _buttons) {
        if (!btn || !btn->isPressed()) {
            allPressed = false;
            break;
        }
    }

    uint32_t now = millis();

    if (allPressed) {
        if (!_isPressed) {
            _isPressed = true;
            _pressStartTime = now;
            _isHolding = false;
            if (_onPress) {
                _onPress();
            }
        } else if (!_isHolding && (now - _pressStartTime >= _holdTimeMs)) {
            _isHolding = true;
            _flagHold = true;
            if (_onHold) {
                _onHold();
            }
        }
    } else {
        if (_isPressed) {
            _isPressed = false;
            if (_onRelease) {
                _onRelease();
            }
            if (!_isHolding) {
                _flagClick = true;
                if (_onClick) {
                    _onClick();
                }
            }
            _isHolding = false;
        }
    }
}
