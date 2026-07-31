#include "ui_overlay/ui_root_layout.h"

#include "layout/layout_manager.h"
#include "ui/ui.h"

namespace {
struct screen_layout_entry_t {
    lv_obj_t *screen;
    lv_obj_t *app;
    lv_obj_t *bottom;
};

screen_layout_entry_t s_entries[32] = {};
uint8_t s_entry_count = 0;

void style_container(lv_obj_t *obj) {
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

void register_entry(lv_obj_t *screen, lv_obj_t *app, lv_obj_t *bottom) {
    if (!screen || !app || s_entry_count >= (sizeof(s_entries) / sizeof(s_entries[0]))) {
        return;
    }
    s_entries[s_entry_count].screen = screen;
    s_entries[s_entry_count].app = app;
    s_entries[s_entry_count].bottom = bottom;
    s_entry_count++;
}

void wrap_screen(lv_obj_t *screen, const layout_spec_t &layout) {
    if (!screen) {
        return;
    }

    uint32_t child_count = lv_obj_get_child_cnt(screen);
    if (child_count > 128U) {
        child_count = 128U;
    }

    lv_obj_t *original_children[128] = {};
    for (uint32_t i = 0; i < child_count; ++i) {
        original_children[i] = lv_obj_get_child(screen, i);
    }

    lv_obj_set_size(screen, layout.app_width, layout.app_height);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *app = lv_obj_create(screen);
    lv_obj_set_pos(app, layout.app_x, layout.app_y);
    lv_obj_set_size(app, layout.app_width, layout.app_height);
    style_container(app);

    const void *bg_img_src = lv_obj_get_style_bg_img_src(screen, LV_PART_MAIN);
    lv_opa_t bg_opa = lv_obj_get_style_bg_opa(screen, LV_PART_MAIN);
    lv_color_t bg_color = lv_obj_get_style_bg_color(screen, LV_PART_MAIN);
    if (bg_img_src) {
        lv_obj_set_style_bg_img_src(app, bg_img_src, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (bg_opa > LV_OPA_MIN) {
        lv_obj_set_style_bg_opa(app, bg_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_obj_set_style_bg_color(app, bg_color, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Screen must stay transparent after wrap; AppContainer is the background owner.
    lv_obj_set_style_bg_img_src(screen, nullptr, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    for (uint32_t i = 0; i < child_count; ++i) {
        if (original_children[i]) {
            lv_obj_set_parent(original_children[i], app);
        }
    }

    lv_obj_t *bottom = nullptr;
    if (layout.bottom_height > 0) {
        bottom = lv_obj_create(screen);
        lv_obj_set_pos(bottom, layout.bottom_x, layout.bottom_y);
        lv_obj_set_size(bottom, layout.bottom_width, layout.bottom_height);
        style_container(bottom);
        lv_obj_set_style_bg_opa(bottom, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    register_entry(screen, app, bottom);
}
}

void ui_root_layout_apply_all(void) {
    const layout_spec_t &layout = LayoutManager::get();
    s_entry_count = 0;

    lv_obj_t *screens[] = {
        ui_ScreenMainGif,
        ui_ScreenWelcome,
        ui_ScreenWIFIConnecting,
        ui_ScreenWIFIDisconnect,
        ui_ScreenExtrude,
        ui_ScreenMove,
        ui_ScreenTemp,
        ui_ScreenSetTemp,
        ui_ScreenSetExtrude,
        ui_ScreenPrinting,
        ui_ScreenHeatingNozzle,
        ui_ScreenHeatingBed,
        ui_ScreenRoller,
        ui_ScreenQRCode,
        ui_ScreenBacklight,
        ui_ScreenDialog,
        ui_ScreenPopup,
        ui_ScreenColorWheel,
        ui_ScreenInfo,
        ui_ScreenTestImg,
        ui_ScreenTestSensor,
    };

    for (uint32_t i = 0; i < (sizeof(screens) / sizeof(screens[0])); ++i) {
        wrap_screen(screens[i], layout);
    }
}

lv_obj_t *ui_root_layout_get_app_container(lv_obj_t *screen) {
    for (uint8_t i = 0; i < s_entry_count; ++i) {
        if (s_entries[i].screen == screen) {
            return s_entries[i].app;
        }
    }
    return screen;
}

lv_obj_t *ui_root_layout_get_bottom_container(lv_obj_t *screen) {
    for (uint8_t i = 0; i < s_entry_count; ++i) {
        if (s_entries[i].screen == screen) {
            return s_entries[i].bottom;
        }
    }
    return nullptr;
}
