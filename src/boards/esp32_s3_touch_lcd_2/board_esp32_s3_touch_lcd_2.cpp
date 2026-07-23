#include "boards/esp32_s3_touch_lcd_2/board_esp32_s3_touch_lcd_2.h"
#include "pinout.h"
#include <Wire.h>
#include <CST816S.h>

#include "boards/common/cst816_helper.h"

#ifdef ESP32_S3_TOUCH_LCD_2

#ifdef I2C0_SUPPORT
extern TwoWire i2c0;
#endif

namespace {
CST816S s_touch = CST816S(CST816S_RST_PIN, CST816S_IRQ_PIN, &i2c0);
touch_event_t s_touch_event = {};
}

void board_esp32_s3_touch_lcd_2_init(void) {
#ifdef I2C0_SUPPORT
    i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_SPEED);
#endif
}

void board_esp32_s3_touch_lcd_2_display_init(void) {
}

void board_esp32_s3_touch_lcd_2_touch_init(void) {
    cst816_helper_configure(s_touch, 0xB6);
}

void board_esp32_s3_touch_lcd_2_backlight_init(void) {
#ifdef LCD_BL_PIN
    pinMode(LCD_BL_PIN, OUTPUT);
#endif
#ifdef LCD_BL_PWM_CHANNEL
    ledcSetup(LCD_BL_PWM_CHANNEL, LCD_BL_PWM_FREQ_HZ, LCD_BL_PWM_RES_BITS);
    ledcAttachPin(LCD_BL_PIN, LCD_BL_PWM_CHANNEL);
#endif
}

void board_esp32_s3_touch_lcd_2_backlight_set(int8_t level) {
    level = constrain(level, 0, 16);
#ifdef LCD_BL_PWM_CHANNEL
    uint32_t duty_max = (1U << LCD_BL_PWM_RES_BITS) - 1U;
    uint32_t duty = (static_cast<uint32_t>(level) * duty_max) / 16U;
    ledcWrite(LCD_BL_PWM_CHANNEL, duty);
#else
    digitalWrite(LCD_BL_PIN, level > 0 ? HIGH : LOW);
#endif
}

bool board_esp32_s3_touch_lcd_2_touch_read(uint16_t *x, uint16_t *y) {
    return cst816_helper_read(s_touch, s_touch_event, x, y);
}

board_info_t board_esp32_s3_touch_lcd_2_get_info(void) {
    board_info_t info = {
        "ESP32-S3-Touch-LCD-2",
        TFT_WIDTH,
        TFT_HEIGHT,
        true,
        false,
        LayoutType::Portrait240x320,
    };
    return info;
}

int16_t board_esp32_s3_touch_lcd_2_battery_percent(void) {
    return -1;
}

#endif
