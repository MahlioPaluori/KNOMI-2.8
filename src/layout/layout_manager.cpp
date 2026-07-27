#include "layout/layout_manager.h"

namespace {
layout_spec_t s_layout = {
    LayoutType::Square240,
    240,
    240,
    0,
    0,
    240,
    240,
    0,
    240,
    240,
    0,
};
}

void LayoutManager::init(void) {
    board_info_t info = board_get_info();
    s_layout.type = info.layout;
    s_layout.root_width = info.width;
    s_layout.root_height = info.height;

    if (info.layout == LayoutType::Portrait240x320 && info.width == 240 && info.height == 320) {
        s_layout.app_x = 0;
        s_layout.app_y = 0;
        s_layout.app_width = 240;
        s_layout.app_height = 240;
        s_layout.bottom_x = 0;
        s_layout.bottom_y = 240;
        s_layout.bottom_width = 240;
        s_layout.bottom_height = 80;
        return;
    }

    s_layout.app_x = 0;
    s_layout.app_y = 0;
    s_layout.app_width = info.width;
    s_layout.app_height = info.height;
    s_layout.bottom_x = 0;
    s_layout.bottom_y = info.height;
    s_layout.bottom_width = info.width;
    s_layout.bottom_height = 0;
}

const layout_spec_t &LayoutManager::get(void) {
    return s_layout;
}
