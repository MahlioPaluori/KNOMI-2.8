#include "boards/knomi/board_knomi.h"
#include "pinout.h"
#include <Wire.h>
#include <CST816S.h>

#include "boards/common/cst816_helper.h"

#if defined(KNOMIV1) || defined(KNOMIV2)

#ifdef I2C0_SUPPORT
extern TwoWire i2c0;
#endif

namespace {
#ifdef CST816S_SUPPORT
CST816S s_touch = CST816S(CST816S_RST_PIN, CST816S_IRQ_PIN, &i2c0);
touch_event_t s_touch_event = {};
#endif

int8_t s_backlight_level = -1;

void aw9346_set_backlight(int8_t level) {
    if (level > 16) {
        level = 16;
    }
    if (level < 0) {
        level = 0;
    }
    if (s_backlight_level == level) {
        return;
    }

    if (level == 0) {
        digitalWrite(LCD_BL_PIN, LOW);
        delay(3);
        s_backlight_level = 0;
        return;
    }

    if (s_backlight_level <= 0) {
        digitalWrite(LCD_BL_PIN, HIGH);
        delayMicroseconds(25);
        s_backlight_level = 16;
    }

    if (s_backlight_level < level) {
        s_backlight_level += 16;
    }

    int8_t pulses = s_backlight_level - level;
    for (int8_t i = 0; i < pulses; ++i) {
        digitalWrite(LCD_BL_PIN, LOW);
        delayMicroseconds(1);
        digitalWrite(LCD_BL_PIN, HIGH);
        delayMicroseconds(1);
    }

    s_backlight_level = level;
}
}

void board_knomi_init(void) {
#ifdef I2C0_SUPPORT
    i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_SPEED);
#endif
}

void board_knomi_display_init(void) {
}

void board_knomi_touch_init(void) {
#ifdef CST816S_SUPPORT
    cst816_helper_configure(s_touch, 0xB4);
#endif
}

void board_knomi_backlight_init(void) {
#ifdef LCD_BL_PIN
    pinMode(LCD_BL_PIN, OUTPUT);
    digitalWrite(LCD_BL_PIN, LOW);
    delay(3);
    s_backlight_level = 0;
#endif
}

void board_knomi_backlight_set(int8_t level) {
#ifdef LCD_BL_PIN
    aw9346_set_backlight(level);
#else
    (void)level;
#endif
}

bool board_knomi_touch_read(uint16_t *x, uint16_t *y) {
#ifdef CST816S_SUPPORT
    return cst816_helper_read(s_touch, s_touch_event, x, y);
#else
    if (x) {
        *x = 0;
    }
    if (y) {
        *y = 0;
    }
    return false;
#endif
}

board_info_t board_knomi_get_info(void) {
    board_info_t info = {"BTT KNOMI", 240, 240,
#ifdef CST816S_SUPPORT
        true,
#else
        false,
#endif
        false, LayoutType::Square240};
    return info;
}

int16_t board_knomi_battery_percent(void) {
    return -1;
}

#endif
