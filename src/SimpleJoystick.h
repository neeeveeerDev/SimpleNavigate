#ifndef SIMPLE_JOYSTICK_H
#define SIMPLE_JOYSTICK_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "SimpleButton.h"

class SimpleJoystick {
public:
    using Callback = std::function<void()>;

    enum class Direction : uint8_t {
        Center,
        Up,
        Down,
        Left,
        Right
    };

    enum class Direction8 : uint8_t {
        Center,
        Up,
        UpRight,
        Right,
        DownRight,
        Down,
        DownLeft,
        Left,
        UpLeft
    };

    SimpleJoystick(uint8_t pinX, uint8_t pinY, int8_t pinBtn = -1, uint8_t activeLevel = LOW);
    ~SimpleJoystick();

    static void tickAll();

    void tick();

    int8_t getX() const;
    int8_t getY() const;
    uint16_t getRawX() const;
    uint16_t getRawY() const;
    uint8_t getMagnitude() const;
    uint16_t getAngle() const;

    Direction getDirection() const;
    Direction8 getDirection8() const;

    void setInvertX(bool invert);
    void setInvertY(bool invert);
    void setSwapAxes(bool swap);
    void setDeadzone(uint8_t percent);
    void setOuterDeadzone(uint8_t percent);
    void setSmoothing(float factor);
    void setExponent(float expo);
    void setResolution(uint16_t maxAdc);
    void calibrateCenter();
    void setRepeat(uint16_t holdMs, uint16_t stepMs);

    void onUp(Callback callback);
    void onDown(Callback callback);
    void onLeft(Callback callback);
    void onRight(Callback callback);
    void onUpLeft(Callback callback);
    void onUpRight(Callback callback);
    void onDownLeft(Callback callback);
    void onDownRight(Callback callback);
    void onFlick(Direction dir, Callback callback);
    void onDoubleFlick(Direction dir, Callback callback);

    SimpleButton& btn();
    SimpleButton& btnUp();
    SimpleButton& btnDown();
    SimpleButton& btnLeft();
    SimpleButton& btnRight();

private:
    friend class SimpleNavigate;

    uint8_t _pinX;
    uint8_t _pinY;
    SimpleButton* _btnStick;
    SimpleButton _dummyButton;

    SimpleButton _btnUp;
    SimpleButton _btnDown;
    SimpleButton _btnLeft;
    SimpleButton _btnRight;

    uint16_t _maxAdc;
    float _centerX;
    float _centerY;
    bool _calibrated;

    bool _invertX;
    bool _invertY;
    bool _swapAxes;
    uint8_t _deadzone;
    uint8_t _outerDeadzone;
    float _smoothing;
    float _exponent;

    float _filteredX;
    float _filteredY;
    int8_t _normX;
    int8_t _normY;
    uint8_t _magnitude;
    uint16_t _angle;

    Direction _currentDir;
    Direction _lastDir;
    Direction8 _currentDir8;
    Direction8 _lastDir8;

    uint32_t _dirHoldTimer;
    uint32_t _dirStepTimer;
    uint16_t _repeatHoldMs;
    uint16_t _repeatStepMs;

    Direction _lastFlickDir;
    uint32_t _flickStartTime;
    uint32_t _lastFlickCompleteTime;
    uint8_t _flickCount;

    Callback _onUp;
    Callback _onDown;
    Callback _onLeft;
    Callback _onRight;
    Callback _onUpLeft;
    Callback _onUpRight;
    Callback _onDownLeft;
    Callback _onDownRight;

    std::vector<std::pair<Direction, Callback>> _flickCallbacks;
    std::vector<std::pair<Direction, Callback>> _doubleFlickCallbacks;

    static std::vector<SimpleJoystick*> _instances;

    int8_t _calculateNormalized(float raw, float center, bool invert);
};

#endif
