#pragma once

#include <Arduino.h>

// Класс для обработки одной кнопки с защитой от дребезга
class Button {
public:
    Button(uint8_t pin, uint8_t mode = INPUT_PULLUP, uint32_t debounce_ms = 100)
        : pin_(pin)
        , debounce_time_(debounce_ms)
        , last_state_(HIGH)
        , last_press_time_(0)
    {
        pinMode(pin_, mode);
    }

    // Проверка, была ли кнопка нажата (с защитой от дребезга)
    bool isPressed() {
        unsigned long current_time = millis();
        bool current_state = digitalRead(pin_) == LOW;
        bool was_pressed = false;

        if (current_state && !last_state_) {
            // Переход из отжатого в нажатое состояние
            if (current_time - last_press_time_ > debounce_time_) {
                was_pressed = true;
                last_press_time_ = current_time;
            }
        }

        last_state_ = current_state;
        return was_pressed;
    }

    // Проверка текущего состояния без дебаунса
    bool isDown() const {
        return digitalRead(pin_) == LOW;
    }

    // Установка времени дебаунса
    void setDebounceTime(uint32_t ms) {
        debounce_time_ = ms;
    }

private:
    const uint8_t pin_;
    uint32_t debounce_time_;
    bool last_state_;
    unsigned long last_press_time_;
};

// Менеджер кнопок для управления несколькими кнопками
class ButtonManager {
public:
    // Пины кнопок
    static constexpr uint8_t BOOT_BUTTON_PIN = 9;  // GPIO9 - кнопка BOOT на ESP32-C6
    // Добавьте здесь другие пины кнопок по мере необходимости

    ButtonManager()
        : boot_button_(BOOT_BUTTON_PIN)
    {
    }

    // Получить кнопку BOOT
    Button& getBootButton() {
        return boot_button_;
    }

    // Обновить состояние всех кнопок (вызывать в цикле)
    void update() {
        // При добавлении новых кнопок, обновляйте их здесь
        boot_button_.isPressed();  // Обновляем внутреннее состояние
    }

private:
    Button boot_button_;
    // Добавьте другие кнопки здесь
};
