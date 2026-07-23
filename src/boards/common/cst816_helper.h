#ifndef CST816_HELPER_H
#define CST816_HELPER_H

#include <Arduino.h>
#include <CST816S.h>

void cst816_helper_configure(CST816S &touch, uint8_t device_id);
bool cst816_helper_read(CST816S &touch, touch_event_t &event, uint16_t *x, uint16_t *y);

#endif
