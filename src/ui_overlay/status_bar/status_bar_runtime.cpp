#include "ui_overlay/status_bar/status_bar_runtime.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "knomi.h"
#include "moonraker.h"
#include "ui_overlay/status_bar/status_bar.h"

namespace {
uint32_t s_next_wifi_update_ms = 0;
uint32_t s_next_clock_update_ms = 0;
int32_t s_last_rssi = INT32_MIN;
wifi_status_t s_last_wifi_status = WIFI_STATUS_INIT;
uint8_t s_last_clock_minute = 255;

const char *printer_state_text(void) {
    if (moonraker.unconnected) {
        return "OFFLINE";
    }
    if (moonraker.unready) {
        return "UNREADY";
    }
    if (moonraker.data.homing) {
        return "HOMING";
    }
    if (moonraker.data.probing) {
        return "PROBING";
    }
    if (moonraker.data.qgling) {
        return "QGL";
    }
    if (moonraker.data.heating_nozzle || moonraker.data.heating_bed) {
        return "HEATING";
    }
    if (moonraker.data.printing) {
        return moonraker.data.pause ? "PAUSED" : "PRINTING";
    }
    return "READY";
}

void update_printer_state(void) {
    static const char *last_state = nullptr;
    const char *state = printer_state_text();
    if (state != last_state) {
        StatusBar::setPrinterState(state);
        last_state = state;
    }
}

void update_wifi_text(void) {
    wifi_status_t wifi_status = wifi_get_connect_status();
    uint32_t now_ms = millis();
    if (wifi_status == s_last_wifi_status && now_ms < s_next_wifi_update_ms) {
        return;
    }

    if (wifi_status == WIFI_STATUS_CONNECTED) {
        int32_t rssi = WiFi.RSSI();
        if (wifi_status != s_last_wifi_status || rssi != s_last_rssi) {
            char wifi_text[20] = {};
            snprintf(wifi_text, sizeof(wifi_text), "WiFi %ld", (long)rssi);
            StatusBar::setWifiText(wifi_text);
            s_last_rssi = rssi;
        }
    } else if (wifi_status != s_last_wifi_status) {
        StatusBar::setWifiText("--");
        s_last_rssi = INT32_MIN;
    }

    s_last_wifi_status = wifi_status;
    s_next_wifi_update_ms = now_ms + 5000;
}

void update_clock_text(void) {
    uint32_t now_ms = millis();
    if (now_ms < s_next_clock_update_ms) {
        return;
    }
    s_next_clock_update_ms = now_ms + 1000;

    time_t now_time = time(nullptr);
    if (now_time <= 0) {
        if (s_last_clock_minute != 254) {
            StatusBar::setClock("--:--");
            s_last_clock_minute = 254;
        }
        return;
    }

    struct tm local_now;
    if (!localtime_r(&now_time, &local_now) || local_now.tm_year < (2020 - 1900)) {
        if (s_last_clock_minute != 254) {
            StatusBar::setClock("--:--");
            s_last_clock_minute = 254;
        }
        return;
    }

    if ((uint8_t)local_now.tm_min != s_last_clock_minute) {
        char clock_text[6] = {};
        snprintf(clock_text, sizeof(clock_text), "%02d:%02d", local_now.tm_hour, local_now.tm_min);
        StatusBar::setClock(clock_text);
        s_last_clock_minute = (uint8_t)local_now.tm_min;
    }
}
}

void status_bar_runtime_init(void) {
    s_next_wifi_update_ms = 0;
    s_next_clock_update_ms = 0;
    s_last_rssi = INT32_MIN;
    s_last_wifi_status = WIFI_STATUS_INIT;
    s_last_clock_minute = 255;

    update_printer_state();
    update_wifi_text();
    update_clock_text();
}

void status_bar_runtime_update(void) {
    update_printer_state();
    update_wifi_text();
    update_clock_text();
}
