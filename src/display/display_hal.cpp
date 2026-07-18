#include "display_hal.h"
#include "display_knomi.h"

void display_init(void) {
#ifdef KNOMIV2
    display_knomi_init();
#endif

#ifdef WAVESHARE28C
    // TODO: display_waveshare_init();
#endif
}

void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
#ifdef KNOMIV2
    display_knomi_flush(disp, area, color_p);
#endif

#ifdef WAVESHARE28C
    // TODO: display_waveshare_flush(disp, area, color_p);
#endif
}

void display_set_brightness(int8_t level) {
#ifdef KNOMIV2
    display_knomi_set_brightness(level);
#endif

#ifdef WAVESHARE28C
    // TODO: display_waveshare_set_brightness(level);
#endif
}

bool display_read_touch(uint16_t *x, uint16_t *y) {
#ifdef KNOMIV2
    return display_knomi_read_touch(x, y);
#endif

#ifdef WAVESHARE28C
    // TODO: return display_waveshare_read_touch(x, y);
#endif

    return false;
}
