#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include <Arduino.h>
#include "boards/board_layer.h"

struct layout_spec_t {
    LayoutType type;
    uint16_t root_width;
    uint16_t root_height;
    uint16_t app_x;
    uint16_t app_y;
    uint16_t app_width;
    uint16_t app_height;
    uint16_t bottom_x;
    uint16_t bottom_y;
    uint16_t bottom_width;
    uint16_t bottom_height;
};

namespace LayoutManager {
void init(void);
const layout_spec_t &get(void);
}

#endif
