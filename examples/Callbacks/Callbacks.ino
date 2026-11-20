#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton button(4);

void setup() {
    Serial.begin(115200);

    button.onPress([]() {
        Serial.println("Press");
    });

    button.onRelease([]() {
        Serial.println("Release");
    });

    button.onClick([]() {
        Serial.println("Single Click");
    });

    button.onDoubleClick([]() {
        Serial.println("Double Click");
    });

    button.onMultiClick([](uint8_t clicks) {
        Serial.print("Multi Click count: ");
        Serial.println(clicks);
    });

    button.onHold([]() {
        Serial.println("Hold triggered");
    });

    button.onStep([]() {
        Serial.println("Step repeat");
    });
}

void loop() {
    button.tick();
}
