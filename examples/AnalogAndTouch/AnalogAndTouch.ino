#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleAnalogKeypad analogPad(36, 60);

#if defined(ESP32)
SimpleTouch touchButton(T0, 15);
#endif

void setup() {
    Serial.begin(115200);

    analogPad.addKey('R', 0);
    analogPad.addKey('U', 140);
    analogPad.addKey('D', 330);
    analogPad.addKey('L', 500);
    analogPad.addKey('S', 740);

    analogPad.key('S').onClick([]() {
        Serial.println("Analog Pad: Select Click");
    });

    analogPad.key('U').onStep([]() {
        Serial.println("Analog Pad: Up Step Repeat");
    });

    analogPad.key('D').onStep([]() {
        Serial.println("Analog Pad: Down Step Repeat");
    });

#if defined(ESP32)
    touchButton.btn().onClick([]() {
        Serial.println("Capacitive Touch Click");
    });

    touchButton.btn().onHold([]() {
        Serial.println("Capacitive Touch Hold");
    });
#endif
}

void loop() {
    SimpleNavigate::tickAll();
}
