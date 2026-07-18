#include "lvgl_hal.h"
#include "display/display_hal.h"
#include "pinout.h"

/* Display flushing */
void usr_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    display_flush(disp, area, color_p);
}

#ifdef CST816S_SUPPORT
void touch_idle_time_clear(void);
void usr_touchpad_read(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    uint16_t x = 0;
    uint16_t y = 0;
    bool touched = display_read_touch(&x, &y);
    if (touched) {
        data->state = LV_INDEV_STATE_PR;
        /*Set the coordinates*/
        data->point.x = x;
        data->point.y = y;
        touch_idle_time_clear();
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}
#endif

void tft_set_backlight(int8_t aw9346_to_light) {
    display_set_brightness(aw9346_to_light);
}

lv_indev_t * ts_cst816s_indev;
void lvgl_hal_init(void) {
    display_init();

    // must static
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *color_buf = (lv_color_t *)LV_MEM_CUSTOM_ALLOC(TFT_WIDTH * TFT_HEIGHT * sizeof(lv_color_t));
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, color_buf, NULL, TFT_WIDTH * TFT_HEIGHT);

    /*Initialize the display*/
    // must static
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = usr_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    // lv_disp_set_rotation(NULL, LV_DISP_ROT_180);

#ifdef CST816S_SUPPORT
    /* touch screen */
    // must static
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);      /*Basic initialization*/
    indev_drv.gesture_limit = 1;
    indev_drv.gesture_min_velocity = 1;
    indev_drv.type = LV_INDEV_TYPE_POINTER;                 /*See below.*/
    indev_drv.read_cb = usr_touchpad_read;              /*See below.*/
    /*Register the driver in LVGL and save the created input device object*/
    ts_cst816s_indev = lv_indev_drv_register(&indev_drv);
#endif

    /* set background color to black (default white) */
    lv_obj_set_style_bg_color(lv_scr_act(), LV_COLOR_MAKE(0, 0, 0), LV_STATE_DEFAULT);
}
