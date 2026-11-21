#ifndef SIMPLE_NAVIGATE_H
#define SIMPLE_NAVIGATE_H

#include "SimpleButton.h"
#include "SimpleEncoder.h"
#include "SimpleJoystick.h"
#include "SimpleMatrix.h"
#include "SimpleAnalogKeypad.h"
#include "SimpleTouch.h"

#if defined(ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

class SimpleNavigate {
public:
    static void tickAll();

#if defined(ESP32)
    static void startBackgroundPoll(uint32_t intervalMs = 5, uint8_t core = 1);
    static void stopBackgroundPoll();
#endif

private:
#if defined(ESP32)
    static TaskHandle_t _pollTaskHandle;
    static void _backgroundPollTask(void* parameter);
#endif
};

#endif
