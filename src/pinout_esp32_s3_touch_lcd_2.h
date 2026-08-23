#ifndef PINOUT_ESP32_S3_TOUCH_LCD_2_H
#define PINOUT_ESP32_S3_TOUCH_LCD_2_H

// Waveshare ESP32-S3-Touch-LCD-2 (ST7789T3 + CST816D)

#define BOOT_PIN 0

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

// Shared I2C bus
#define I2C0_SUPPORT
#define I2C0_SPEED 400000
#define I2C0_SCL_PIN 47
#define I2C0_SDA_PIN 48

// ST7789T3 SPI pins
#define ST7789_MOSI_PIN 38
#define ST7789_SCLK_PIN 39
#define ST7789_MISO_PIN 40
#define ST7789_CS_PIN 45
#define ST7789_DC_PIN 42
#define ST7789_RST_PIN -1

// Backlight
#define LCD_BL_PIN 1

// CST816D touch (TP_INT is wired to GPIO46; TP_RESET is not MCU-controlled)
#define CST816S_SUPPORT
#define CST816S_IRQ_PIN 46
#define CST816S_RST_PIN -1

// External divider from USB VBUS to GPIO18 (LOW=battery, HIGH=USB present)
#define VBUS_SENSE_PIN 18

// Touch coordinate rotation (0=no rotation, 1=90°, 2=180°, 3=270°)
#define CST816S_ROTATION 0

#endif
