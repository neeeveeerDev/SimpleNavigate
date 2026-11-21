#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleEncoder enc(4, 5, 18);
SimpleJoystick joy(34, 35, 19);

void setup() {
    Serial.begin(115200);

    enc.setRange(0, 100, true);
    enc.setAcceleration(true, 50, 5);

    enc.onLeft([]() {
        Serial.println("Encoder: Left");
    });

    enc.onRight([]() {
        Serial.println("Encoder: Right");
    });

    enc.onLeftHold([]() {
        Serial.println("Encoder: Left + Button Held (Fast Scroll)");
    });

    enc.onRightHold([]() {
        Serial.println("Encoder: Right + Button Held (Fast Scroll)");
    });

    enc.onChange([](int16_t delta, int16_t pos) {
        Serial.print("Encoder Value: ");
        Serial.println(pos);
    });

    enc.btn().onClick([]() {
        Serial.println("Encoder Button: Single Click");
    });

    enc.btn().onDoubleClick([]() {
        Serial.println("Encoder Button: Double Click");
    });

    enc.btn().onHold([]() {
        Serial.println("Encoder Button: Hold");
    });

    joy.setDeadzone(10);
    joy.setExponent(1.8f);

    joy.onUp([]() {
        Serial.println("Joystick: D-Pad UP");
    });

    joy.onDown([]() {
        Serial.println("Joystick: D-Pad DOWN");
    });

    joy.onLeft([]() {
        Serial.println("Joystick: D-Pad LEFT");
    });

    joy.onRight([]() {
        Serial.println("Joystick: D-Pad RIGHT");
    });

    joy.onFlick(SimpleJoystick::Direction::Right, []() {
        Serial.println("Joystick Gesture: Quick Flick RIGHT!");
    });

    joy.btn().onClick([]() {
        Serial.println("Joystick Stick Click");
    });

#if defined(ESP32)
    SimpleNavigate::startBackgroundPoll(5, 1);
#endif
}

void loop() {
#if !defined(ESP32)
    SimpleNavigate::tickAll();
#endif
}
