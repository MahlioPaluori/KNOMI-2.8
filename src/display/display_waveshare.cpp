#include "display_waveshare.h"
#include "pinout.h"

void display_waveshare_init(void) {
    // Placeholder for Waveshare backend initialization.
    // Hardware setup is intentionally left for a future commit.
}

void display_waveshare_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    (void)disp;
    (void)area;
    (void)color_p;
}

void display_waveshare_set_brightness(int8_t level) {
    (void)level;
}

bool display_waveshare_read_touch(uint16_t *x, uint16_t *y) {
    if (x) *x = 0;
    if (y) *y = 0;
    return false;
}
