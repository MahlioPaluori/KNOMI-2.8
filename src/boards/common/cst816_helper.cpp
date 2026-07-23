#include "boards/common/cst816_helper.h"

void cst816_helper_configure(CST816S &touch, uint8_t device_id) {
    touch.begin(CST816S_DEFAULT_ADDRESS, FALLING, device_id);
    touch.setReportRate(2);
    touch.setReportMode(0x60);
    touch.setMotionMask(0);
    touch.setAutoRst(0);
    touch.setLongRst(0);
    touch.setDisAutoSleep(1);
}

bool cst816_helper_read(CST816S &touch, touch_event_t &event, uint16_t *x, uint16_t *y) {
    if (touch.ready()) {
        touch.getTouch(&event);
    }

    if (event.finger) {
        if (x) {
            *x = event.x;
        }
        if (y) {
            *y = event.y;
        }
        return true;
    }

    if (x) {
        *x = 0;
    }
    if (y) {
        *y = 0;
    }
    return false;
}
