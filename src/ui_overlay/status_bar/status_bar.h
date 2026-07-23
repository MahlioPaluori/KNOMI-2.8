#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <Arduino.h>

namespace StatusBar {
void init(void);
void setBattery(int8_t percent);
void setWifi(int8_t rssi);
void setPrinter(const char *state);
void setTime(const char *hhmm);
void setCharging(bool charging);
}

#endif
