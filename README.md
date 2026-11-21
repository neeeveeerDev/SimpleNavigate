# SimpleNavigate
[![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)](https://github.com/NEEEVEEER/SimpleNavigate)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%7C%20PlatformIO-orange.svg)](https://platformio.org/)
[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP8266%20%7C%20AVR%20%7C%20STM32%20%7C%20RP2040-green.svg)](https://github.com/NEEEVEEER/SimpleNavigate)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

SimpleNavigate — лёгкая и неблокирующая библиотека для работы с тактовыми кнопками, ацп кнопками, энкодерами, джойстиками , матричными клавиратурами на микроконтроллерах ESP32, ESP8266, Arduino и других совместимых платформах

---

## Версии

### `[SimpleButton]`
* **v1.0.0** — Релиз ядра для тактовых кнопок (антидребезг, клики, мультиклик, удержание, автоповтор).
* **v1.1.0** — FreeRTOS опрос для ESP32, комбинации кнопок, режим тумблера, защита от залипания и Deep Sleep.

---
🔄 **Переименование:** `SimpleButton` ➔ **`SimpleNavigate`**
---

### `[SimpleNavigate]`
* **v2.0.0** — Масштабное расширение в библиотеку навигации: добавлены энкодеры, матричные клавиатуры, аналоговые джойстики, АЦП-кнопки, I2C-расширители и сенсорные пины ESP32.

---

## Оглавление

1. [Возможности библиотеки](#возможности-библиотеки)
2. [Установка](#установка)
3. [Быстрый старт](#быстрый-старт)
4. [Модуль SimpleButton (Кнопки)](#1-модуль-simplebutton-кнопки)
5. [Модуль SimpleEncoder (Энкодеры)](#2-модуль-simpleencoder-энкодеры)
6. [Модуль SimpleJoystick (Джойстики)](#3-модуль-simplejoystick-джойстики)
7. [Модуль SimpleMatrix (Матричные клавиатуры)](#4-модуль-simplematrix-матричные-клавиатуры)
8. [Модуль SimpleAnalogKeypad (Резистивные АЦП-кнопки)](#5-модуль-simpleanalogkeypad-резистивные-ацп-кнопки)
9. [Модуль SimpleTouch (Сенсоры ESP32)](#6-модуль-simpletouch-сенсоры-esp32)
10. [Фоновый опрос FreeRTOS (ESP32)](#7-фоновый-опрос-freertos-esp32)
11. [Лицензия](#лицензия)

---

## Возможности библиотеки

- **Инициализация в одну строку**: никаких сложных структур, конфигураций и лишнего кода.
- **Событийная модель C++11**: назначение лямбд прямо в `setup()`.
- **Нулевые задержки**: библиотека построена на конечных автоматах (FSM) и `millis()`, без единого `delay()`.
- **Единая экосистема**: каждая клавиша матрицы, кнопка джойстика, АЦП-кнопка или сенсор работает через ядро `SimpleButton`.
- **Централизованный диспетчер**: команда `SimpleNavigate::tickAll()` опрашивает все подключенные устройства разом.
- **Многозадачность FreeRTOS**: фоновый опрос на отдельном ядре ESP32 с нулевой нагрузкой на `loop()`.

---

## Установка

### PlatformIO
Скопируйте папку библиотеки в каталог `lib/` проекта:
```ini
; platformio.ini
lib_deps =
    SimpleNavigate
```

### Arduino IDE
Поместите папку библиотеки в каталог `libraries/` в папке с вашими скетчами Arduino.

---

## Быстрый старт

```cpp
#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleButton btn(4);

void setup() {
    Serial.begin(115200);

    btn.onClick([]() {
        Serial.println("Click");
    });

    btn.onDoubleClick([]() {
        Serial.println("Double Click");
    });

    btn.onHold([]() {
        Serial.println("Hold");
    });
}

void loop() {
    SimpleNavigate::tickAll();
}
```

---

## 1. Модуль SimpleButton (Кнопки)

### Создание объекта
```cpp
SimpleButton(uint8_t pin, uint8_t activeLevel = LOW, uint8_t pinConfig = INPUT_PULLUP);
SimpleButton(CustomReader reader);
```

### События (коллбэки)
```cpp
btn.onPress([]() { });
btn.onRelease([]() { });
btn.onClick([]() { });
btn.onDoubleClick([]() { });
btn.onMultiClick([](uint8_t count) { });
btn.onHold([]() { });
btn.onHoldFor(3000, []() { });
btn.onStep([]() { });
btn.onToggle([](bool state) { });
btn.onStuck([]() { });
```

### Проверка состояний (для loop)
```cpp
bool isPressed();
bool hasPressed();
bool hasReleased();
bool isClick();
bool isDouble();
bool isHold();
bool isStep();
bool isToggled();
bool isStuck();
uint8_t getClicks();
uint32_t getHoldTime();
```

### Тайминги и параметры
```cpp
btn.setDebounce(40);
btn.setHoldTime(600);
btn.setStepTime(150);
btn.setClickTimeout(300);
btn.setStepAcceleration(30, 5);
btn.setStuckTimeout(15000);
```

### Комбинации кнопок (`SimpleButtonCombo`)
```cpp
SimpleButton btnA(4);
SimpleButton btnB(5);
SimpleButtonCombo combo(btnA, btnB);

void setup() {
    combo.onHold([]() {
        Serial.println("Both buttons held");
    });
}
```

---

## 2. Модуль SimpleEncoder (Энкодеры)

### Создание объекта
```cpp
SimpleEncoder enc(uint8_t pinA, uint8_t pinB, int8_t pinBtn = -1, uint8_t activeLevel = LOW, uint8_t pinConfig = INPUT_PULLUP);
```

### Возможности
- Табличный квадратурный декодер с защитой от пропуска шагов и дребезга.
- Повороты с одновременным удержанием кнопки вала (`onLeftHold`, `onRightHold`).
- Адаптивное ускорение вращения (`isFast`).
- Ограничение диапазона и зацикливание (`setRange`).

### Пример
```cpp
#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleEncoder enc(4, 5, 18);

void setup() {
    Serial.begin(115200);

    enc.setRange(0, 100, true);
    enc.setAcceleration(true, 50, 5);

    enc.onLeft([]() {
        Serial.println("Left");
    });

    enc.onRight([]() {
        Serial.println("Right");
    });

    enc.onLeftHold([]() {
        Serial.println("Left + Button Held");
    });

    enc.onRightHold([]() {
        Serial.println("Right + Button Held");
    });

    enc.btn().onClick([]() {
        Serial.println("Encoder Button Click");
    });
}

void loop() {
    SimpleNavigate::tickAll();
}
```

---

## 3. Модуль SimpleJoystick (Джойстики)

### Создание объекта
```cpp
SimpleJoystick joy(uint8_t pinX, uint8_t pinY, int8_t pinBtn = -1, uint8_t activeLevel = LOW);
```

### Возможности
- Нормализованные координаты от -100% до +100%.
- Внутренняя и внешняя мертвые зоны (`setDeadzone`, `setOuterDeadzone`).
- Фильтрация шумов АЦП (EMA) и экспонента отклика (`setExponent`).
- Векторная математика: `getMagnitude()` (0..100%) и `getAngle()` (0..360°).
- 4-позиционный и 8-позиционный D-Pad с автоповтором при удержании.
- Распознавание хлестких рывков (жесты Flick и Double Flick).

### Пример
```cpp
#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleJoystick joy(34, 35, 19);

void setup() {
    Serial.begin(115200);

    joy.setDeadzone(10);
    joy.setOuterDeadzone(5);
    joy.setExponent(1.8f);

    joy.onUp([]() {
        Serial.println("Up");
    });

    joy.onDown([]() {
        Serial.println("Down");
    });

    joy.onFlick(SimpleJoystick::Direction::Right, []() {
        Serial.println("Flick Right");
    });

    joy.btn().onClick([]() {
        Serial.println("Stick Click");
    });
}

void loop() {
    SimpleNavigate::tickAll();
}
```

---

## 4. Модуль SimpleMatrix (Матричные клавиатуры)

### Создание объекта
```cpp
SimpleMatrix keypad(rowPins, colPins, layout);
```

### Возможности
- Сканирование матриц 3x4, 4x4, NxM без блокировки процессора.
- Глобальное событие нажатия `onKeyPress` / `onKeyRelease`.
- Доступ к любой физической клавише как к `SimpleButton`: метод `key(char id)`.

### Пример
```cpp
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

    keypad.onKeyPress([](char k) {
        Serial.print("Pressed: ");
        Serial.println(k);
    });

    keypad.key('#').onHold([]() {
        Serial.println("# Held for 600ms");
    });

    keypad.key('*').onDoubleClick([]() {
        Serial.println("* Double Click");
    });
}

void loop() {
    SimpleNavigate::tickAll();
}
```

---

## 5. Модуль SimpleAnalogKeypad (Резистивные АЦП-кнопки)

### Возможности
- Чтение нескольких кнопок на одном аналоговом пине через резистивный делитель.
- Сглаживание шума и программный допуск по напряжению (`setTolerance`).
- Каждая кнопка ведет себя как независимый объект `SimpleButton`.

### Пример
```cpp
#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleAnalogKeypad analogPad(36, 50);

void setup() {
    Serial.begin(115200);

    analogPad.addKey('R', 0);
    analogPad.addKey('U', 140);
    analogPad.addKey('D', 330);
    analogPad.addKey('L', 500);
    analogPad.addKey('S', 740);

    analogPad.key('S').onClick([]() {
        Serial.println("Select Click");
    });

    analogPad.key('U').onStep([]() {
        Serial.println("Up Auto-Repeat");
    });
}

void loop() {
    SimpleNavigate::tickAll();
}
```

---

## 6. Модуль SimpleTouch (Сенсоры ESP32)

### Возможности
- Прямая работа с аппаратными емкостными сенсорными пинами ESP32 (`touchRead`).
- Автокалибровка фоновой емкости при запуске.
- Доступ к кнопке через `touch.btn()` со всеми кликами и удержаниями.

### Пример
```cpp
#include <Arduino.h>
#include <SimpleNavigate.h>

#if defined(ESP32)
SimpleTouch touchBtn(T0, 15);
#endif

void setup() {
    Serial.begin(115200);

#if defined(ESP32)
    touchBtn.btn().onClick([]() {
        Serial.println("Touch Click");
    });

    touchBtn.btn().onHold([]() {
        Serial.println("Touch Hold");
    });
#endif
}

void loop() {
    SimpleNavigate::tickAll();
}
```

---

## 7. Фоновый опрос FreeRTOS (ESP32)

На микроконтроллерах ESP32 библиотека позволяет полностью освободить `loop()` от вызовов опроса. Задача создается на отдельном ядре с интервалом 5 мс:

```cpp
#include <Arduino.h>
#include <SimpleNavigate.h>

SimpleButton btn(4);
SimpleEncoder enc(5, 18, 19);

void setup() {
    Serial.begin(115200);

    btn.onClick([]() {
        Serial.println("Button Click");
    });

    enc.onLeft([]() {
        Serial.println("Encoder Left");
    });

#if defined(ESP32)
    SimpleNavigate::startBackgroundPoll(5, 1);
#endif
}

void loop() {
}
```

---

## Лицензия

Распространяется под свободной лицензией MIT.  
Разработчик: **NEEEVEEER**

