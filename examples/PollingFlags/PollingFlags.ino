#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton btn(4);

void setup() {
    Serial.begin(115200);
}

void loop() {
    btn.tick();

    if (btn.isClick()) {
        Serial.println("Flag: Click");
    }

    if (btn.isDouble()) {
        Serial.println("Flag: Double");
    }

    if (btn.isHold()) {
        Serial.println("Flag: Hold");
    }

    if (btn.isStep()) {
        Serial.println("Flag: Step");
    }

    uint8_t clicks = btn.getClicks();
    if (clicks > 2) {
        Serial.print("Flag: Clicks = ");
        Serial.println(clicks);
    }
}
