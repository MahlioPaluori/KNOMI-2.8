#include "ui_overlay/status_bar/status_bar_runtime.h"

#include <Arduino.h>
#include <WiFi.h>

#include "boards/board_layer.h"
#include "knomi.h"
#include "moonraker.h"
#include "ui_overlay/status_bar/status_bar.h"

namespace {
uint32_t s_last_update_ms = 0;
static constexpr uint32_t kStatusUpdatePeriodMs = 1000;

const char *printer_state_text(void) {
    if (moonraker.unconnected) {
        return "Offline";
    }
    if (moonraker.unready) {
        return "Ready?";
    }
    if (moonraker.data.printing) {
        return "Printing";
    }
    if (moonraker.data.homing) {
        return "Homing";
    }
    if (moonraker.data.probing) {
        return "Probing";
    }
    return "Idle";
}

void update_time(void) {
    uint32_t total_seconds = millis() / 1000U;
    uint8_t hh = static_cast<uint8_t>((total_seconds / 3600U) % 24U);
    uint8_t mm = static_cast<uint8_t>((total_seconds / 60U) % 60U);
    char time_text[6] = {};
    snprintf(time_text, sizeof(time_text), "%02u:%02u", hh, mm);
    StatusBar::setTime(time_text);
}
}

void status_bar_runtime_init(void) {
    s_last_update_ms = 0;
    StatusBar::setBattery(board_battery_percent());
    StatusBar::setWifi(0);
    StatusBar::setPrinter("Idle");
    StatusBar::setCharging(false);
    StatusBar::setTime("--:--");
}

void status_bar_runtime_update(void) {
    uint32_t now = millis();
    if (now - s_last_update_ms < kStatusUpdatePeriodMs) {
        return;
    }
    s_last_update_ms = now;

    int8_t rssi = 0;
    if (wifi_get_connect_status() == WIFI_STATUS_CONNECTED) {
        rssi = static_cast<int8_t>(WiFi.RSSI());
    }

    StatusBar::setBattery(board_battery_percent());
    StatusBar::setWifi(rssi);
    StatusBar::setPrinter(printer_state_text());
    StatusBar::setCharging(false);
    update_time();
}
