#include "SimpleNavigate.h"

#if defined(ESP32)
TaskHandle_t SimpleNavigate::_pollTaskHandle = nullptr;

void SimpleNavigate::_backgroundPollTask(void* parameter) {
    uint32_t interval = (parameter != nullptr) ? *reinterpret_cast<uint32_t*>(parameter) : 5;
    delete reinterpret_cast<uint32_t*>(parameter);

    TickType_t delayTicks = pdMS_TO_TICKS(interval);
    if (delayTicks == 0) {
        delayTicks = 1;
    }

    while (true) {
        SimpleNavigate::tickAll();
        vTaskDelay(delayTicks);
    }
}

void SimpleNavigate::startBackgroundPoll(uint32_t intervalMs, uint8_t core) {
    if (_pollTaskHandle != nullptr) {
        return;
    }
    uint32_t* intervalParam = new uint32_t(intervalMs);
    xTaskCreatePinnedToCore(
        _backgroundPollTask,
        "SimpleNavigateTask",
        4096,
        intervalParam,
        1,
        &_pollTaskHandle,
        core
    );
}

void SimpleNavigate::stopBackgroundPoll() {
    if (_pollTaskHandle != nullptr) {
        vTaskDelete(_pollTaskHandle);
        _pollTaskHandle = nullptr;
    }
}
#endif

void SimpleNavigate::tickAll() {
    SimpleButton::tickAll();
    SimpleEncoder::tickAll();
    SimpleJoystick::tickAll();
    SimpleMatrix::tickAll();
    SimpleAnalogKeypad::tickAll();
    SimpleTouch::tickAll();
}
