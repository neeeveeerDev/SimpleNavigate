#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton btnPower(4);
SimpleButton btnVolUp(5);
SimpleButton btnVolDown(18);

SimpleButtonCombo comboReset(btnVolUp, btnVolDown);

void setup() {
    Serial.begin(115200);

    btnPower.onToggle([](bool state) {
        Serial.print("Power state: ");
        Serial.println(state ? "ON" : "OFF");
    });

    btnPower.setStuckTimeout(15000);
    btnPower.onStuck([]() {
        Serial.println("Warning: Power button is stuck!");
    });

#if defined(ESP32)
    btnPower.enableWakeup();
#endif

    comboReset.onHold([]() {
        Serial.println("Combo: VolUp + VolDown held for 600ms -> Factory Reset");
    });

#if defined(ESP32)
    SimpleButton::startBackgroundPoll(5, 1);
#endif
}

void loop() {
#if !defined(ESP32)
    SimpleButton::tickAll();
#endif

    if (btnPower.hasPressed()) {
        Serial.println("Power contact made");
    }

    if (btnPower.hasReleased()) {
        Serial.println("Power contact broken");
    }
}
