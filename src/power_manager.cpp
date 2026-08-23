#include "power_manager.h"

#include <Arduino.h>

#include "knomi.h"

#ifdef ESP32_S3_TOUCH_LCD_2

#include <esp_sleep.h>
#include "driver/gpio.h"
#include <lvgl.h>

#include "lvgl_hal.h"
#include "pinout.h"
#include "ui/ui.h"

namespace {
constexpr uint32_t BATTERY_SLEEP_TIMEOUT_MS = 60000;
constexpr uint32_t WIFI_SUSPEND_TIMEOUT_MS = 1500;

uint32_t s_last_user_activity_ms = 0;
bool s_usb_present = false;
bool s_initialized = false;
bool s_sleeping = false;
int8_t s_saved_backlight = 16;

bool usb_present(void) {
    return digitalRead(VBUS_SENSE_PIN) == HIGH;
}

void clear_wake_configuration(void) {
    esp_err_t clear18 = gpio_wakeup_disable(static_cast<gpio_num_t>(VBUS_SENSE_PIN));
    esp_err_t clear46 = gpio_wakeup_disable(static_cast<gpio_num_t>(CST816S_IRQ_PIN));
    esp_err_t clear_global = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
    if (clear18 != ESP_OK || clear46 != ESP_OK || clear_global != ESP_OK) {
        Serial.printf("[PWR] wake cleanup error=%d/%d/%d\n", clear18, clear46, clear_global);
    }
}

void restore_after_sleep(esp_sleep_wakeup_cause_t wake_cause, int gpio18, int gpio46) {
    // Restore active state and visible feedback before network recovery.
    s_sleeping = false;
    s_usb_present = gpio18 == HIGH;
    s_last_user_activity_ms = millis();
    tft_set_backlight(s_saved_backlight);

    clear_wake_configuration();
    pinMode(VBUS_SENSE_PIN, INPUT);
    pinMode(CST816S_IRQ_PIN, INPUT);

    lv_obj_invalidate(lv_scr_act());
    lv_obj_invalidate(lv_layer_top());
    lv_refr_now(nullptr);

    wifi_power_resume();

    bool woke_for_usb = gpio18 == HIGH;
    bool woke_for_touch = wake_cause == ESP_SLEEP_WAKEUP_GPIO && !woke_for_usb && gpio46 == LOW;
    Serial.printf("[PWR] wake=%s usb=%u touch=%u\n",
                  wake_cause == ESP_SLEEP_WAKEUP_GPIO ? "gpio" : "other",
                  woke_for_usb ? 1U : 0U,
                  woke_for_touch ? 1U : 0U);
}

void abort_sleep_preparation(bool restore_backlight) {
    clear_wake_configuration();
    if (restore_backlight) {
        tft_set_backlight(s_saved_backlight);
    }
    wifi_power_resume();
    s_last_user_activity_ms = millis();
}

void try_enter_light_sleep(void) {
    if (s_sleeping || usb_present() || digitalRead(CST816S_IRQ_PIN) == LOW) {
        return;
    }

    // Every cycle starts clean and rebuilds both wake sources explicitly.
    clear_wake_configuration();

    esp_err_t vbus_wake = gpio_wakeup_enable(
        static_cast<gpio_num_t>(VBUS_SENSE_PIN), GPIO_INTR_HIGH_LEVEL);
    esp_err_t touch_wake = gpio_wakeup_enable(
        static_cast<gpio_num_t>(CST816S_IRQ_PIN), GPIO_INTR_LOW_LEVEL);
    esp_err_t gpio_wake = esp_sleep_enable_gpio_wakeup();
    if (vbus_wake != ESP_OK || touch_wake != ESP_OK || gpio_wake != ESP_OK) {
        Serial.printf("[PWR] sleep abort: wake config %d/%d/%d\n",
                      vbus_wake, touch_wake, gpio_wake);
        abort_sleep_preparation(false);
        return;
    }

    if (!wifi_power_suspend(WIFI_SUSPEND_TIMEOUT_MS)) {
        Serial.println("[PWR] sleep abort: wifi");
        abort_sleep_preparation(false);
        return;
    }

    if (usb_present() || digitalRead(CST816S_IRQ_PIN) == LOW) {
        abort_sleep_preparation(false);
        return;
    }

    s_saved_backlight = static_cast<int8_t>(lv_slider_get_value(ui_slider_backlight));
    tft_set_backlight(0);

    if (usb_present() || digitalRead(CST816S_IRQ_PIN) == LOW) {
        abort_sleep_preparation(true);
        return;
    }

    s_sleeping = true;
    Serial.println("[PWR] sleep enter");

    // Native USB Serial must not be flushed without a host; it can block before sleep.
    esp_err_t sleep_result = esp_light_sleep_start();
    esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();
    int gpio18 = digitalRead(VBUS_SENSE_PIN);
    int gpio46 = digitalRead(CST816S_IRQ_PIN);
    if (sleep_result != ESP_OK) {
        Serial.printf("[PWR] sleep error=%d\n", sleep_result);
    }
    restore_after_sleep(wake_cause, gpio18, gpio46);
}
} // namespace

void power_manager_init(void) {
    pinMode(VBUS_SENSE_PIN, INPUT);
    pinMode(CST816S_IRQ_PIN, INPUT);
    s_usb_present = usb_present();
    s_last_user_activity_ms = millis();
    s_sleeping = false;
    s_initialized = true;
    Serial.printf("[PWR] mode=%s\n", s_usb_present ? "usb" : "battery");
}

void power_manager_notify_user_activity(void) {
    if (s_initialized) {
        s_last_user_activity_ms = millis();
    }
}

void power_manager_update(void) {
    if (!s_initialized || s_sleeping) {
        return;
    }

    uint32_t now_ms = millis();
    bool now_usb_present = usb_present();
    if (now_usb_present != s_usb_present) {
        s_usb_present = now_usb_present;
        s_last_user_activity_ms = now_ms;
        Serial.printf("[PWR] mode=%s\n", s_usb_present ? "usb" : "battery");
    }

    if (s_usb_present) {
        return;
    }

    if (static_cast<uint32_t>(now_ms - s_last_user_activity_ms) < BATTERY_SLEEP_TIMEOUT_MS) {
        return;
    }

    try_enter_light_sleep();
}

#else

void power_manager_init(void) {}
void power_manager_notify_user_activity(void) {}
void power_manager_update(void) {}

#endif
