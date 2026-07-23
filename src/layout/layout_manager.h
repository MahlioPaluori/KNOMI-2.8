#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include <Arduino.h>
#include "boards/board_layer.h"

struct layout_spec_t {
    LayoutType type;
    uint16_t display_width;
    uint16_t display_height;
    uint16_t content_x;
    uint16_t content_y;
    uint16_t content_width;
    uint16_t content_height;
    uint16_t status_bar_x;
    uint16_t status_bar_y;
    uint16_t status_bar_width;
    uint16_t status_bar_height;
};

namespace LayoutManager {
void init(void);
const layout_spec_t &get(void);
}

#endif
