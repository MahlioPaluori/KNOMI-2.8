#include "boards/board_layer.h"

#if defined(ESP32_S3_TOUCH_LCD_2)
#include "boards/esp32_s3_touch_lcd_2/board_esp32_s3_touch_lcd_2.h"
#elif defined(WAVESHARE28C)
#include "boards/waveshare_28c/board_waveshare_28c.h"
#elif defined(KNOMIV1) || defined(KNOMIV2)
#include "boards/knomi/board_knomi.h"
#endif

void board_init(void) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    board_esp32_s3_touch_lcd_2_init();
#elif defined(WAVESHARE28C)
    board_waveshare_28c_init();
#elif defined(KNOMIV1) || defined(KNOMIV2)
    board_knomi_init();
#endif
}

void board_display_init(void) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    board_esp32_s3_touch_lcd_2_display_init();
#elif defined(WAVESHARE28C)
    board_waveshare_28c_display_init();
#elif defined(KNOMIV1) || defined(KNOMIV2)
    board_knomi_display_init();
#endif
}

void board_touch_init(void) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    board_esp32_s3_touch_lcd_2_touch_init();
#elif defined(WAVESHARE28C)
    board_waveshare_28c_touch_init();
#elif defined(KNOMIV1) || defined(KNOMIV2)
    board_knomi_touch_init();
#endif
}

void board_backlight_init(void) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    board_esp32_s3_touch_lcd_2_backlight_init();
#elif defined(WAVESHARE28C)
    board_waveshare_28c_backlight_init();
#elif defined(KNOMIV1) || defined(KNOMIV2)
    board_knomi_backlight_init();
#endif
}

void board_backlight_set(int8_t level) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    board_esp32_s3_touch_lcd_2_backlight_set(level);
#elif defined(WAVESHARE28C)
    board_waveshare_28c_backlight_set(level);
#elif defined(KNOMIV1) || defined(KNOMIV2)
    board_knomi_backlight_set(level);
#else
    (void)level;
#endif
}

bool board_touch_read(uint16_t *x, uint16_t *y) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    return board_esp32_s3_touch_lcd_2_touch_read(x, y);
#elif defined(WAVESHARE28C)
    return board_waveshare_28c_touch_read(x, y);
#elif defined(KNOMIV1) || defined(KNOMIV2)
    return board_knomi_touch_read(x, y);
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

board_info_t board_get_info(void) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    return board_esp32_s3_touch_lcd_2_get_info();
#elif defined(WAVESHARE28C)
    return board_waveshare_28c_get_info();
#elif defined(KNOMIV1) || defined(KNOMIV2)
    return board_knomi_get_info();
#else
    board_info_t info = {"unknown", 240, 240, false, false, LayoutType::Square240};
    return info;
#endif
}

int16_t board_battery_percent(void) {
#if defined(ESP32_S3_TOUCH_LCD_2)
    return board_esp32_s3_touch_lcd_2_battery_percent();
#elif defined(WAVESHARE28C)
    return board_waveshare_28c_battery_percent();
#elif defined(KNOMIV1) || defined(KNOMIV2)
    return board_knomi_battery_percent();
#else
    return -1;
#endif
}
