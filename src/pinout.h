#ifndef PINOUT_H
#define PINOUT_H

#ifdef WAVESHARE28C

#include "pinout_waveshare_28c.h"

#elif defined(ESP32_S3_TOUCH_LCD_2)

#include "pinout_esp32_s3_touch_lcd_2.h"

#elif defined(KNOMIV2)

#include "pinout_knomi_v2.h"

#else

#include "pinout_knomi_v1.h"

#endif

#endif