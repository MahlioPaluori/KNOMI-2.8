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
    s_layout.display_width = info.width;
    s_layout.display_height = info.height;

    if (info.layout == LayoutType::Portrait240x320 && info.width == 240 && info.height == 320) {
        s_layout.content_x = 0;
        s_layout.content_y = 0;
        s_layout.content_width = 240;
        s_layout.content_height = 240;
        s_layout.status_bar_x = 0;
        s_layout.status_bar_y = 240;
        s_layout.status_bar_width = 240;
        s_layout.status_bar_height = 80;
        return;
    }

    s_layout.content_x = 0;
    s_layout.content_y = 0;
    s_layout.content_width = info.width;
    s_layout.content_height = info.height;
    s_layout.status_bar_x = 0;
    s_layout.status_bar_y = info.height;
    s_layout.status_bar_width = info.width;
    s_layout.status_bar_height = 0;
}

const layout_spec_t &LayoutManager::get(void) {
    return s_layout;
}
