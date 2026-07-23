#include "display_hal.h"
#include "display_knomi.h"
#include "display_waveshare.h"
#include "display_esp32_s3_touch_lcd_2.h"

void display_init(void) {
#if defined(KNOMIV1) || defined(KNOMIV2)
    display_knomi_init();
#endif

#ifdef WAVESHARE28C
    display_waveshare_init();
#endif

#ifdef ESP32_S3_TOUCH_LCD_2
    display_esp32_s3_touch_lcd_2_init();
#endif
}

void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
#if defined(KNOMIV1) || defined(KNOMIV2)
    display_knomi_flush(disp, area, color_p);
#endif

#ifdef WAVESHARE28C
    display_waveshare_flush(disp, area, color_p);
#endif

#ifdef ESP32_S3_TOUCH_LCD_2
    display_esp32_s3_touch_lcd_2_flush(disp, area, color_p);
#endif
}

void display_set_brightness(int8_t level) {
#if defined(KNOMIV1) || defined(KNOMIV2)
    display_knomi_set_brightness(level);
#endif

#ifdef WAVESHARE28C
    display_waveshare_set_brightness(level);
#endif

#ifdef ESP32_S3_TOUCH_LCD_2
    display_esp32_s3_touch_lcd_2_set_brightness(level);
#endif
}

bool display_read_touch(uint16_t *x, uint16_t *y) {
#if defined(KNOMIV1) || defined(KNOMIV2)
    return display_knomi_read_touch(x, y);
#endif

#ifdef WAVESHARE28C
    return display_waveshare_read_touch(x, y);
#endif

#ifdef ESP32_S3_TOUCH_LCD_2
    return display_esp32_s3_touch_lcd_2_read_touch(x, y);
#endif

    return false;
}
