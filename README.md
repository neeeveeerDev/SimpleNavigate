# SimpleButton
SimpleButton — лёгкая и неблокирующая библиотека для работы с тактовыми кнопками на микроконтроллерах ESP32, ESP8266, Arduino и других совместимых платформах

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

## Особенности

- Инициализация любой кнопки в 1 строку: `SimpleButton btn(4);`
- Событийная модель на базе C++11 лямбд прямо в `setup()`
- Классический режим опроса через флаги внутри `loop()`
- Встроенная машина состояний (FSM) без задержек `delay()`
- Аппаратная фильтрация дребезга контактов (debounce)
- Поддержка одиночного, двойного и N-кратного клика (мультиклик)
- Ступенчатое удержание кнопки (`onHoldFor`) под разные интервалы времени
- Автоповтор (`onStep`) с динамическим ускорением при удержании
- Поддержка виртуальных, матричных и аналоговых кнопок через пользовательскую функцию чтения
- Групповой опрос всех созданных кнопок одной командой `SimpleButton::tickAll()`

---

## Установка

### PlatformIO
Поместите папку `SimpleButton` в каталог `lib/` вашего проекта.

### Arduino IDE
Скопируйте папку `SimpleButton` в каталог `libraries/` в папке с вашими скетчами Arduino.

---

## Быстрый старт

```cpp
#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton button(4);

void setup() {
    Serial.begin(115200);

    button.onPress([]() {
        Serial.println("Press");
    });

    button.onClick([]() {
        Serial.println("Single Click");
    });

    button.onDoubleClick([]() {
        Serial.println("Double Click");
    });

    button.onHold([]() {
        Serial.println("Hold");
    });
}

void loop() {
    button.tick();
}
```

---

## Документация API

### Конструкторы

```cpp
SimpleButton(uint8_t pin, uint8_t activeLevel = LOW, uint8_t pinConfig = INPUT_PULLUP);
SimpleButton(CustomReader reader);
```

- По умолчанию пин настраивается как `INPUT_PULLUP` с активным уровнем `LOW`.
- Для виртуальных и аналоговых кнопок передается лямбда, возвращающая `bool`.

### События (коллбэки)

```cpp
void onPress(Callback callback);
void onRelease(Callback callback);
void onClick(Callback callback);
void onDoubleClick(Callback callback);
void onMultiClick(MultiCallback callback);
void onHold(Callback callback);
void onHoldFor(uint16_t durationMs, Callback callback);
void onStep(Callback callback);
```

### Опрос флагов (классический стиль)

```cpp
bool isPressed() const;
bool isClick();
bool isDouble();
bool isHold();
bool isStep();
uint8_t getClicks();
uint32_t getHoldTime() const;
```

Флаги `isClick()`, `isDouble()`, `isHold()` и `isStep()` являются однократными и сбрасываются после чтения или на следующем вызове `tick()`.

### Настройки таймингов

```cpp
void setDebounce(uint16_t ms);
void setHoldTime(uint16_t ms);
void setStepTime(uint16_t ms);
void setClickTimeout(uint16_t ms);
void setStepAcceleration(uint16_t minIntervalMs, uint16_t stepDecayMs);
```

| Параметр | По умолчанию | Описание |
| :--- | :--- | :--- |
| `setDebounce` | 40 мс | Время фильтрации дребезга контактов |
| `setHoldTime` | 600 мс | Время нажатия для перехода в удержание |
| `setStepTime` | 150 мс | Базовый интервал автоповтора при удержании |
| `setClickTimeout` | 300 мс | Время ожидания следующего клика в серии |
| `setStepAcceleration` | отключено | Минимальный интервал и шаг уменьшения задержки |

### Управление группой и сброс

```cpp
static void tickAll();
void reset();
```

---

## Продвинутые примеры

### Ступенчатое удержание под системные команды

```cpp
#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton button(4);

void setup() {
    Serial.begin(115200);

    button.onHoldFor(1000, []() {
        Serial.println("1 sec: Action");
    });

    button.onHoldFor(3000, []() {
        Serial.println("3 sec: Menu");
    });

    button.onHoldFor(10000, []() {
        Serial.println("10 sec: Reboot");
    });
}

void loop() {
    button.tick();
}
```

### Групповой опрос и виртуальная кнопка

```cpp
#include <Arduino.h>
#include <SimpleButton.h>

SimpleButton btnA(4);
SimpleButton btnB(5);
SimpleButton btnAnalog([]() {
    return analogRead(A0) > 512;
});

void setup() {
    Serial.begin(115200);

    btnA.onClick([]() {
        Serial.println("A Click");
    });

    btnB.setStepAcceleration(30, 5);
    btnB.onStep([]() {
        Serial.println("Fast Step");
    });

    btnAnalog.onClick([]() {
        Serial.println("Analog Click");
    });
}

void loop() {
    SimpleButton::tickAll();
}
```

---

## Лицензия

MIT License. Автор: NEEEVEEER
