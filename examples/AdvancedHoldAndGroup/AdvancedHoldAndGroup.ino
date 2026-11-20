#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton btnPrimary(4);
SimpleButton btnSecondary(5);
SimpleButton btnAnalog([]() {
    return analogRead(A0) > 512;
});

void setup() {
    Serial.begin(115200);

    btnPrimary.onHoldFor(1000, []() {
        Serial.println("Hold 1 sec: Quick Action");
    });

    btnPrimary.onHoldFor(3000, []() {
        Serial.println("Hold 3 sec: Settings Mode");
    });

    btnPrimary.onHoldFor(10000, []() {
        Serial.println("Hold 10 sec: Factory Reset");
    });

    btnSecondary.setStepAcceleration(30, 10);
    btnSecondary.onStep([]() {
        Serial.println("Accelerating Step");
    });

    btnAnalog.onClick([]() {
        Serial.println("Analog Virtual Click");
    });
}

void loop() {
    SimpleButton::tickAll();
}
