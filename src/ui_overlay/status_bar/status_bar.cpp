#include "ui_overlay/status_bar/status_bar.h"

#include <stdio.h>
#include <lvgl.h>

#include "layout/layout_manager.h"

namespace {
lv_obj_t *s_root = nullptr;
lv_obj_t *s_battery = nullptr;
lv_obj_t *s_wifi = nullptr;
lv_obj_t *s_printer = nullptr;
lv_obj_t *s_time = nullptr;
lv_obj_t *s_charging = nullptr;

void set_label_text(lv_obj_t *label, const char *txt) {
    if (!label) {
        return;
    }
    lv_label_set_text(label, txt ? txt : "");
}
}

void StatusBar::init(void) {
    const layout_spec_t &layout = LayoutManager::get();
    if (layout.status_bar_height == 0) {
        return;
    }

    s_root = lv_obj_create(lv_layer_top());
    lv_obj_set_size(s_root, layout.status_bar_width, layout.status_bar_height);
    lv_obj_set_pos(s_root, layout.status_bar_x, layout.status_bar_y);
    lv_obj_set_style_radius(s_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(s_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(s_root, LV_OPA_90, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(s_root, lv_color_make(18, 18, 18), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(s_root, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    s_battery = lv_label_create(s_root);
    lv_obj_align(s_battery, LV_ALIGN_LEFT_MID, 0, 0);

    s_wifi = lv_label_create(s_root);
    lv_obj_align(s_wifi, LV_ALIGN_LEFT_MID, 70, 0);

    s_printer = lv_label_create(s_root);
    lv_obj_align(s_printer, LV_ALIGN_CENTER, 0, 0);

    s_time = lv_label_create(s_root);
    lv_obj_align(s_time, LV_ALIGN_RIGHT_MID, -48, 0);

    s_charging = lv_label_create(s_root);
    lv_obj_align(s_charging, LV_ALIGN_RIGHT_MID, 0, 0);

    set_label_text(s_battery, "BAT --%");
    set_label_text(s_wifi, "WiFi --");
    set_label_text(s_printer, "Idle");
    set_label_text(s_time, "--:--");
    set_label_text(s_charging, "");
}

void StatusBar::setBattery(int8_t percent) {
    if (!s_battery) {
        return;
    }
    char text[16] = {};
    if (percent < 0) {
        snprintf(text, sizeof(text), "BAT --%%");
    } else {
        snprintf(text, sizeof(text), "BAT %d%%", percent);
    }
    set_label_text(s_battery, text);
}

void StatusBar::setWifi(int8_t rssi) {
    if (!s_wifi) {
        return;
    }
    char text[16] = {};
    if (rssi == 0) {
        snprintf(text, sizeof(text), "WiFi --");
    } else {
        snprintf(text, sizeof(text), "WiFi %d", rssi);
    }
    set_label_text(s_wifi, text);
}

void StatusBar::setPrinter(const char *state) {
    set_label_text(s_printer, state ? state : "Idle");
}

void StatusBar::setTime(const char *hhmm) {
    set_label_text(s_time, hhmm ? hhmm : "--:--");
}

void StatusBar::setCharging(bool charging) {
    set_label_text(s_charging, charging ? "CHG" : "");
}
