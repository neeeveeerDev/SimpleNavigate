#include <Arduino.h>
#include <SimpleNavigate.h>

std::vector<uint8_t> rowPins = {12, 13, 14, 15};
std::vector<uint8_t> colPins = {16, 17, 18, 19};

std::vector<std::vector<char>> layout = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

SimpleMatrix keypad(rowPins, colPins, layout);

void setup() {
    Serial.begin(115200);

    keypad.onKeyPress([](char key) {
        Serial.print("Matrix Key Pressed: ");
        Serial.println(key);
    });

    keypad.key('#').onHold([]() {
        Serial.println("Lock System: # held for 600ms");
    });

    keypad.key('*').onDoubleClick([]() {
        Serial.println("Menu Mode: * double-clicked");
    });

    keypad.key('0').onHoldFor(3000, []() {
        Serial.println("Factory Reset: 0 held for 3 sec");
    });
}

void loop() {
    SimpleNavigate::tickAll();
}
