#include <Arduino.h>
#include <Wire.h>
#include "lvgl_hal.h"
#include "pinout.h"
#include "boards/board_layer.h"
#include "layout/layout_manager.h"

#include "ui/ui.h"


// Hardware init
#ifdef I2C0_SUPPORT
TwoWire i2c0 = TwoWire(0);
// TwoWire i2c1 = TwoWire(1);
#endif

void lvgl_ui_task(void * parameter);
void lis2dw12_task(void * parameter);
void wifi_task(void * parameter);
void moonraker_task(void * parameter);
void setup() {
    Serial.begin(115200);
    Serial0.begin(115200);
    delay(50);
    Serial.println("\r\n\r\n------------- Knomi startup -----------\r\n");
    Serial0.println("\r\n\r\n------------- Knomi startup -----------\r\n");
    Serial.println("SPI Flash: ");
    Serial0.println("SPI Flash: ");
    Serial.print("  Size: ");
    Serial.println(ESP.getFlashChipSize());
    Serial0.print("  Size: ");
    Serial0.println(ESP.getFlashChipSize());
    Serial.print("  Speed: ");
    Serial.println(ESP.getFlashChipSpeed());
    Serial0.print("  Speed: ");
    Serial0.println(ESP.getFlashChipSpeed());
    Serial.print("  Mode: ");
    Serial.println(ESP.getFlashChipMode());
    Serial0.print("  Mode: ");
    Serial0.println(ESP.getFlashChipMode());
    Serial.println("SPI PSRAM: ");
    Serial0.println("SPI PSRAM: ");
    Serial.print("  Found: ");
    Serial.println(psramFound());
    Serial0.print("  Found: ");
    Serial0.println(psramFound());
    Serial.print("  Size: ");
    Serial.println(ESP.getPsramSize());
    Serial0.print("  Size: ");
    Serial0.println(ESP.getPsramSize());
    Serial.println("\r\n\r\n------------------------------------------\r\n");
    Serial0.println("\r\n\r\n------------------------------------------\r\n");

    board_init();
    LayoutManager::init();

    xTaskCreate(lvgl_ui_task, "lvgl ui",
        4096,  // Stack size (bytes)
        NULL,  // Parameter to pass
        10,     // Task priority
        NULL   // Task handle
        );

#ifdef LIS2DW_SUPPORT
    xTaskCreate(lis2dw12_task, "lis2dw12",
        4096,  // Stack size (bytes)
        NULL,  // Parameter to pass
        9,     // Task priority
        NULL   // Task handle
        );
#endif

    xTaskCreate(wifi_task, "wifi",
        4096,  // Stack size (bytes)
        NULL,  // Parameter to pass
        8,     // Task priority
        NULL   // Task handle
        );

    xTaskCreate(moonraker_task, "moonraker",
        4096,  // Stack size (bytes)
        NULL,  // Parameter to pass
        7,     // Task priority
        NULL   // Task handle
        );
}

void loop() {
}
