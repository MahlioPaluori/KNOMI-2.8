#include "boards/esp32_s3_touch_lcd_2/board_esp32_s3_touch_lcd_2.h"
#include "pinout.h"
#include <Wire.h>

#ifdef ESP32_S3_TOUCH_LCD_2

#ifdef I2C0_SUPPORT
extern TwoWire i2c0;
#endif

// CST816D touch controller I2C registers (Waveshare approach - polling only, no interrupts)
#define CST816_ADDR             0x15
#define CST816_ID_REG           0xA7
#define CST816_TOUCH_NUM_REG    0x02
#define CST816_TOUCH_XH_REG     0x03
#define CST816_TOUCH_XL_REG     0x04
#define CST816_TOUCH_YH_REG     0x05
#define CST816_TOUCH_YL_REG     0x06

namespace {
static uint16_t g_touchpad_x = 0;
static uint16_t g_touchpad_y = 0;
}

void board_esp32_s3_touch_lcd_2_init(void) {
#ifdef I2C0_SUPPORT
    i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_SPEED);
#endif
}

void board_esp32_s3_touch_lcd_2_display_init(void) {
}

void board_esp32_s3_touch_lcd_2_touch_init(void) {
    // Verify CST816D is present on I2C by reading device ID
    uint8_t id = 0;
    i2c0.beginTransmission(CST816_ADDR);
    i2c0.write(CST816_ID_REG);
    i2c0.endTransmission(true);
    i2c0.requestFrom(CST816_ADDR, 1);
    if (i2c0.available()) {
        id = i2c0.read();
    }
    Serial.printf("CST816D Device ID: 0x%02X (expect 0xB6)\n", id);
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
    uint8_t touch_num = 0;
    uint8_t touch_raw[2];
    
    // Read touch count
    i2c0.beginTransmission(CST816_ADDR);
    i2c0.write(CST816_TOUCH_NUM_REG);
    i2c0.endTransmission(true);
    i2c0.requestFrom(CST816_ADDR, 1);
    if (i2c0.available()) {
        touch_num = i2c0.read();
    }
    
    if (touch_num == 0) {
        return false;
    }
    
    // Read X coordinates (high byte)
    i2c0.beginTransmission(CST816_ADDR);
    i2c0.write(CST816_TOUCH_XH_REG);
    i2c0.endTransmission(true);
    i2c0.requestFrom(CST816_ADDR, 1);
    touch_raw[0] = i2c0.available() ? i2c0.read() : 0;
    
    // Read X coordinates (low byte)
    i2c0.beginTransmission(CST816_ADDR);
    i2c0.write(CST816_TOUCH_XL_REG);
    i2c0.endTransmission(true);
    i2c0.requestFrom(CST816_ADDR, 1);
    touch_raw[1] = i2c0.available() ? i2c0.read() : 0;
    
    g_touchpad_x = (uint16_t)((touch_raw[0] & 0x0F) << 8) | touch_raw[1];
    
    // Read Y coordinates (high byte)
    i2c0.beginTransmission(CST816_ADDR);
    i2c0.write(CST816_TOUCH_YH_REG);
    i2c0.endTransmission(true);
    i2c0.requestFrom(CST816_ADDR, 1);
    touch_raw[0] = i2c0.available() ? i2c0.read() : 0;
    
    // Read Y coordinates (low byte)
    i2c0.beginTransmission(CST816_ADDR);
    i2c0.write(CST816_TOUCH_YL_REG);
    i2c0.endTransmission(true);
    i2c0.requestFrom(CST816_ADDR, 1);
    touch_raw[1] = i2c0.available() ? i2c0.read() : 0;
    
    g_touchpad_y = (uint16_t)((touch_raw[0] & 0x0F) << 8) | touch_raw[1];
    
    // Apply coordinate rotation based on CST816S_ROTATION parameter
    uint16_t touch_x = 0, touch_y = 0;
    
    switch (CST816S_ROTATION) {
        case 1:
            // 90° rotation: swap X/Y and invert X
            touch_x = g_touchpad_y;
            touch_y = TFT_HEIGHT - 1 - g_touchpad_x;
            break;
        case 2:
            // 180° rotation: invert both
            touch_x = TFT_WIDTH - 1 - g_touchpad_x;
            touch_y = TFT_HEIGHT - 1 - g_touchpad_y;
            break;
        case 3:
            // 270° rotation: swap X/Y and invert Y
            touch_x = TFT_WIDTH - 1 - g_touchpad_y;
            touch_y = g_touchpad_x;
            break;
        default:
            // Case 0: no rotation (identity)
            touch_x = g_touchpad_x;
            touch_y = g_touchpad_y;
            break;
    }
    
    if (x) *x = touch_x;
    if (y) *y = touch_y;
    
    return true;
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
