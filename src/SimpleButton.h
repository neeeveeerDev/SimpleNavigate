#ifndef SIMPLE_BUTTON_H
#define SIMPLE_BUTTON_H

#include <Arduino.h>
#include <functional>
#include <vector>

class SimpleButton {
public:
    using Callback = std::function<void()>;
    using MultiCallback = std::function<void(uint8_t)>;
    using CustomReader = std::function<bool()>;

    explicit SimpleButton(uint8_t pin, uint8_t activeLevel = LOW, uint8_t pinConfig = INPUT_PULLUP);
    explicit SimpleButton(CustomReader reader);
    ~SimpleButton();

    static void tickAll();

    void tick();
    void reset();

    void onPress(Callback callback);
    void onRelease(Callback callback);
    void onClick(Callback callback);
    void onDoubleClick(Callback callback);
    void onMultiClick(MultiCallback callback);
    void onHold(Callback callback);
    void onHoldFor(uint16_t durationMs, Callback callback);
    void onStep(Callback callback);

    bool isPressed() const;
    bool isClick();
    bool isDouble();
    bool isHold();
    bool isStep();
    uint8_t getClicks();
    uint32_t getHoldTime() const;

    void setDebounce(uint16_t ms);
    void setHoldTime(uint16_t ms);
    void setStepTime(uint16_t ms);
    void setClickTimeout(uint16_t ms);
    void setStepAcceleration(uint16_t minIntervalMs, uint16_t stepDecayMs);

private:
    enum class State : uint8_t {
        Idle,
        DebouncePress,
        Pressed,
        Holding,
        DebounceRelease,
        WaitNextClick
    };

    struct HoldThreshold {
        uint16_t duration;
        Callback callback;
        bool triggered;
    };

    uint8_t _pin;
    uint8_t _activeLevel;
    uint8_t _pinConfig;
    bool _configured;
    CustomReader _reader;

    State _state;
    uint32_t _stateTimer;
    uint32_t _pressStartTime;
    uint32_t _lastStepTime;
    uint32_t _lastHoldTime;
    uint8_t _clickCount;
    uint8_t _lastReturnedClicks;

    bool _debouncedState;
    bool _flagClick;
    bool _flagDouble;
    bool _flagHold;
    bool _flagStep;

    uint16_t _debounceMs;
    uint16_t _holdTimeMs;
    uint16_t _stepTimeMs;
    uint16_t _clickTimeoutMs;
    uint16_t _minStepIntervalMs;
    uint16_t _stepDecayMs;
    uint16_t _currentStepIntervalMs;

    Callback _onPress;
    Callback _onRelease;
    Callback _onClick;
    Callback _onDoubleClick;
    MultiCallback _onMultiClick;
    Callback _onHold;
    Callback _onStep;
    std::vector<HoldThreshold> _holdThresholds;

    static std::vector<SimpleButton*> _instances;

    bool _readRaw();
};

#endif
