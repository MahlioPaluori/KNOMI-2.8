# Project context

This document describes the current working-tree architecture. It is not a generic KNOMI manual and does not imply that planned features are implemented.

## Project purpose

KNOMI firmware provides a touch-driven LVGL interface for Klipper printers and reads/controls printer state through Moonraker. The original interface targets 240x240 KNOMI displays. Current development adapts it to the Waveshare ESP32-S3-Touch-LCD-2's 240x320 portrait display, retaining the 240x240 application while adding a persistent 240x80 StatusBar. Board-specific battery sleep and wake are hardware-verified; orientation work remains planned.

## Primary hardware

The active target is `ESP32_S3_TOUCH_LCD_2`, selected by PlatformIO environment `esp32_s3_touch_lcd_2`.

| Component | Verified current detail | Status / evidence |
|---|---|---|
| MCU | ESP32-S3 at 240 MHz | **IMPLEMENTED**; `buildroot/boards/esp32s3r8.json` |
| Flash | 16 MB, QIO, 80 MHz | **IMPLEMENTED**; board JSON and `spiffs_16MB.csv` |
| PSRAM | Board definition identifies 8 MB, `qio_opi`, `BOARD_HAS_PSRAM` | **IMPLEMENTED**; board JSON |
| Display | ST7789/ST7789T3, 240x320 | **IMPLEMENTED**; pinout and `tft_setup.h` |
| Display SPI | MOSI 38, SCLK 39, MISO 40, CS 45, DC 42, RST `-1` | **IMPLEMENTED**; active pinout |
| Backlight | GPIO 1; current backend is on/off because no PWM-channel macros are active | **PARTIAL** |
| Touch | CST816D, I2C `0x15` | **IMPLEMENTED**, polling; board backend expects device ID `0xB6` |
| Shared I2C | SDA 48, SCL 47, 400 kHz | **IMPLEMENTED** |
| Battery | ADC GPIO 5, assumed divider multiplier 3.0 | **PARTIAL**; derived from production definitions and Waveshare battery examples |
| Motion sensor | QMI8658 at vendor-reference address `0x6B`, acceleration + gyro | **REFERENCE ONLY**; absent from production firmware |
| Touch IRQ/reset wiring | TP_INT GPIO46; TP_RESET is not MCU-controlled | **HARDWARE VERIFIED**; active-low interrupt pulses return HIGH after touch activity |
| USB detection | VBUS sense GPIO18; external 100 kOhm / 147 kOhm divider | **HARDWARE VERIFIED**; LOW=battery, HIGH=USB present |

## Build architecture

`platformio.ini` uses Arduino on `espressif32@6.4.0` and defines:

| Environment | Board | Target define |
|---|---|---|
| `esp32_s3_touch_lcd_2` (default/primary) | `esp32s3r8` | `ESP32_S3_TOUCH_LCD_2` |
| `waveshare28c` | `esp32s3r8` | `WAVESHARE28C` |
| `knomiv2` | `esp32s3r8` | `KNOMIV2` |
| `knomiv1` | `esp32r8` | `KNOMIV1` |

Primary build command:

```powershell
platformio run -e esp32_s3_touch_lcd_2
```

Common build details:

- `src` is an include path and `src/tft_setup.h` is force-included.
- `LV_CONF_PATH` selects `include/lv_conf.h`.
- USB CDC on boot is enabled.
- `spiffs_16MB.csv` provides NVS, OTA metadata, two 4.5 MB application slots, SPIFFS, and coredump storage.
- Core libraries include LVGL 8.3.7, TFT_eSPI 2.5.0, ArduinoJson 6.19.x, ESPAsyncWebServer, AsyncElegantOTA, LIS2DW12, and SHT4X.
- LIS2DW12 and SHT4X dependencies do not mean those sensors are active on the primary board; their feature defines are absent.

## Runtime startup architecture

Production entry is `src/main.cpp -> setup()`:

```text
Arduino startup
-> Serial and Serial0 at 115200; flash/PSRAM diagnostics
-> board_init()
   -> initialize shared I2C on SDA 48/SCL 47
-> LayoutManager::init()
-> create LVGL task (priority 10)
-> omit LIS2DW12 task for this target
-> create WiFi task (priority 8)
-> create Moonraker GET task (priority 7)
   -> create Moonraker POST task (priority 8)
```

Task creation is sequential, but task execution is concurrent; there is no single total initialization order after tasks begin.

`src/lvgl_usr.cpp -> lvgl_ui_task()` initializes:

```text
button support
-> lvgl_hal_init()
   -> display/TFT/touch/backlight
   -> LVGL and full-frame draw buffer
   -> 240x320 display driver
   -> pointer input driver
-> SquareLine ui_init()
-> ui_root_layout_apply_all()
-> StatusBar::create(lv_layer_top())
-> status_bar_runtime_init()
-> runtime GIF/QR/version/roller objects
-> 5 ms UI loop
```

The WiFi task loads configuration, configures AP/STA mode, scanning, DNS, web server/OTA, and status. After a successful station connection it starts NTP. The Moonraker task only polls when WiFi reports connected.

## LVGL/UI architecture

```text
+------------------------+
|                        |
|       KNOMI UI         |
|        240x240         |
|                        |
+------------------------+
|       StatusBar        |
|        240x80          |
+------------------------+
```

`src/layout/layout_manager.cpp -> LayoutManager::init()` defines a 240x320 root, application rectangle `(0,0,240,240)`, and bottom rectangle `(0,240,240,80)` for this board.

SquareLine-generated screens remain LVGL screen objects. `src/ui_overlay/ui_root_layout.cpp -> wrap_screen()`:

1. Captures each screen's generated children.
2. Sets the screen to the 240x240 application size.
3. Creates a transparent 240x240 application container.
4. Copies the screen background image/opacity/color to that container.
5. Clears the screen's own background and reparents generated children into the application container.
6. Creates a transparent per-screen bottom container at y=240.

The per-screen bottom containers are registered but currently do not own the persistent bar. Runtime-created UI objects use `ui_root_layout_get_app_container()` when they must belong to the application region.

Navigation remains the SquareLine helper `_ui_screen_change()`, which calls `lv_scr_load_anim()`. The StatusBar is created once on display-owned `lv_layer_top()`, outside normal screen ownership, so screen animations do not move it.

## Important LVGL lessons

### Screen backgrounds

Generated background styles belong to their screen object. Expanding an original 240x240 screen to 240x320 centers its background image in the larger object. The wrapper instead keeps the application at 240x240, moves the background to the application container, and avoids generated-screen edits.

### Display background

Normal screens cover only the upper 240 pixels. The lower uncovered area exposes the LVGL display background, which is distinct from `lv_layer_top()`. The display background is configured separately in `lvgl_hal_init()`.

### TopLayer

`lv_layer_top()` belongs to the display and is independent of normal screen loads. This is why it is appropriate for the persistent StatusBar.

### Style reset ordering

`lv_obj_remove_style_all()` removes style-backed geometry as well as visual styles. Persistent custom containers must reset styles before applying required size/alignment. The reverse ordering previously made the StatusBar root collapse to roughly 130x130 instead of 240x80; current `StatusBar::create()` deliberately resets first.

## Display pipeline

```text
LVGL
-> usr_disp_flush()
-> display_flush()
-> display_esp32_s3_touch_lcd_2_flush()
-> TFT_eSPI
-> ST7789
-> SPI
```

Current known-good configuration:

- Resolution: 240x320.
- TFT rotation: 0; LVGL display rotation is not enabled.
- Color: RGB565, `LV_COLOR_DEPTH=16`, `LV_COLOR_16_SWAP=0`.
- Controller order: `TFT_RGB_ORDER TFT_BGR`. BGR is known-good and must be preserved.
- Inversion: `TFT_INVERSION_ON`.
- Flush: address window then `pushColors(..., true)`, enabling byte swapping at the TFT_eSPI call.
- SPI frequency: 40 MHz; read frequency 5 MHz.
- Buffer: one full 240x320 `lv_color_t` frame allocated through LVGL's custom PSRAM allocator (`ps_malloc`).

## Touch pipeline

```text
CST816D physical touch
-> board_esp32_s3_touch_lcd_2_touch_read()
-> display_read_touch()
-> usr_touchpad_read()
-> LVGL pointer/gesture recognition
-> generated LV_EVENT_GESTURE callbacks
-> _ui_screen_change()
-> lv_scr_load_anim()
```

The production backend polls finger count and X/Y registers over I2C and applies compile-time coordinate rotation (`CST816S_ROTATION`, currently 0). LVGL derives swipe gestures from pointer motion; production does not read the controller gesture register. TP_INT is wired to GPIO46. Hardware capture verified a HIGH idle level with active-low pulses during touch activity and a reliable return to HIGH afterward. GPIO46 LOW-level light-sleep wake is implemented and hardware-verified across repeated and mixed wake cycles.

The local generic `lib/CST816S` reference exposes controller gesture IDs for click, double-click, long press and swipes; motion-mask/report-mode controls; auto-sleep; IRQ handling; and reset-based wake. GPIO46 interrupt signaling is verified, but edge counts did not distinguish double tap from other touch activity. Production still polls touch and does not read the gesture register. TP_RESET remains unavailable to firmware.

## StatusBar architecture

`src/ui_overlay/status_bar/status_bar.cpp -> StatusBar::create()` currently creates:

```text
lv_layer_top()
`- StatusBarRoot: 100% x 80, bottom aligned, opaque black, row flex
   |- left
   |  `- statusMessageLabel (WiFi)
   |- leftCenter (empty)
   |- center (flex-grow)
   |  `- printerStateLabel (layer)
   |- rightCenter (empty)
   `- right
      `- clockLabel (now battery)
```

The bar is a passive view. `status_bar_runtime.cpp` reads runtime state and calls setters; rendering code contains no HTTP requests, WiFi polling, or ADC acquisition. Its top-layer parent makes it stationary during `lv_scr_load_anim()` transitions.

### WiFi

`status_bar_runtime_update() -> update_wifi_text()` reads `wifi_get_connect_status()`. It always displays `WiFi`, green when connected and red otherwise. The UI loop calls it about every 5 ms; color is only reset on connection-state changes, although the text setter currently runs each loop.

### Layer

Intended format is `current/total`, for example `35/175`. The data path is:

```text
Moonraker HTTP
-> MOONRAKER::get_progress()
-> moonraker.data.{current_layer,total_layers,has_layer_info}
-> status_bar_runtime_update() / update_layer_text()
-> StatusBar::setPrinterState()
-> printerStateLabel
```

The UI samples each loop and only rewrites the label when cached values change. This path is **WORKING and HARDWARE VERIFIED** with OrcaSlicer-provided `SET_PRINT_STATS_INFO` values. With no valid layer data it displays `--/--`.

### Battery

The right label uses LVGL battery symbols and recolor markup:

- 76-100%: full icon.
- 56-75%: three-quarter icon.
- 36-55%: half icon.
- 16-35%: one-quarter icon.
- 0-15%: empty icon.
- The icon is green on battery and blue when GPIO18 reports USB present. Percentage text remains white.
- A charging display exists in the view API, but this board has no verified charge-status define and does not select it.

## Moonraker architecture

Transport is HTTP; no WebSocket path exists. `MOONRAKER moonraker` and `moonraker_data_t` in `src/moonraker.h` are the central runtime state.

```text
Moonraker HTTP
-> MOONRAKER::http_get_loop()
-> moonraker.data
-> status_bar_runtime_update()
-> StatusBar setters
-> LVGL labels
```

The GET loop runs every 200 ms while WiFi is connected:

1. `/printer/objects/query?webhooks` -> Klipper readiness.
2. `/printer/objects/query?gcode_macro%20_KNOMI_STATUS` -> homing, probing, QGL, nozzle heating, and bed heating.
3. `/api/printer` -> printing/cancelling, pause, bed temperature, and configured-tool temperature.
4. While printing, `/printer/objects/query?virtual_sdcard&print_stats` -> progress, file path, and layer fields.

`data_unlock` brackets GET refresh for the main UI, but the StatusBar reads the shared structure directly without a mutex/snapshot. GET failures mark the connection state; JSON deserialization errors are not explicitly checked.

POST commands use a five-entry ring buffer. A separate task checks it every 500 ms and calls the same HTTP request method. Request timeout is 60 seconds.

## Layer indicator

**STATUS: WORKING / HARDWARE VERIFIED**

During printing, the center StatusBar displays `current_layer/total_layer`, for example `35/175`. The verified production source is:

- `result.status.print_stats.info.current_layer`
- `result.status.print_stats.info.total_layer`

The complete verified path is:

```text
OrcaSlicer
-> generated G-code containing SET_PRINT_STATS_INFO
-> Klipper
-> Moonraker print_stats.info
-> MOONRAKER::get_progress()
-> moonraker.data
-> status_bar_runtime_update()
-> StatusBar center label
```

OrcaSlicer must emit both counters. The configuration confirmed working for this project is:

Machine Start G-code:

```gcode
SET_PRINT_STATS_INFO TOTAL_LAYER=[total_layer_count]
```

Layer Change G-code:

```gcode
SET_PRINT_STATS_INFO CURRENT_LAYER={layer_num + 1}
```

The former `--/--` problem was not caused by StatusBar rendering, LVGL, parser type handling, or StatusBar runtime updates. Hardware diagnostics showed that `print_stats.info` existed but both layer fields were null because OrcaSlicer was not emitting `SET_PRINT_STATS_INFO`. Once those commands were added to generated G-code, the existing firmware pipeline displayed real layer values correctly.

`MOONRAKER::get_progress()` retains metadata and progress-derived fallback behavior. If the slicer does not provide layer commands and no fallback produces valid information, firmware clears `has_layer_info` and displays `--/--`. This is expected missing-data behavior, not a StatusBar failure. It also clears layer state when no print is active, preventing stale values from a previous print.

## Battery implementation

**STATUS: PARTIAL**

`status_bar_runtime.cpp` implements this board-specific path:

```text
GPIO 5
-> analogReadMilliVolts(), 12 immediate samples
-> remove one minimum and one maximum
-> average remaining 10 samples
-> millivolts to pin volts
-> multiply by BAT_ADC_DIVIDER_RATIO (3.0)
-> piecewise LiPo curve with linear interpolation
-> StatusBar icon/percentage
```

The reading runs immediately during StatusBar initialization and every 5 seconds thereafter. The curve is:

| Voltage | Percent |
|---:|---:|
| 4.20 V | 100 |
| 4.10 V | 90 |
| 4.00 V | 80 |
| 3.90 V | 65 |
| 3.80 V | 45 |
| 3.70 V | 25 |
| 3.60 V | 10 |
| 3.50 V | 5 |
| 3.40 V | 0 |

Limitations:

- State of charge is voltage-derived, not measured by a fuel gauge.
- There is no long-term filter, load compensation, or board-specific calibration coefficient beyond calibrated millivolts and the assumed divider.
- Charging state remains unavailable. USB/VBUS presence is independently detected through GPIO18, not inferred from battery voltage.
- GPIO18 uses an external divider with R1=100 kOhm and R2=147 kOhm (100 kOhm + 47 kOhm); measured levels are approximately 0 V without USB and 2.65 V with USB.
- `board_get_info()` reports `has_battery=false`, and `board_battery_percent()` returns `-1`; the StatusBar currently bypasses that abstraction and directly samples the ADC.

## Power management

**STATUS: HARDWARE VERIFIED**

For `esp32_s3_touch_lcd_2`, firmware enters light sleep after 60 seconds without touch while on battery, regardless of idle, printing, paused, or heating state. It suspends WiFi through the WiFi task, turns off only the backlight, and leaves the LVGL object tree and active screen in memory. On wake it immediately resets active state and the inactivity timestamp, restores the previous brightness, invalidates/repaints the active screen and top layer, and requests WiFi reconnection. External USB power inhibits automatic sleep.

GPIO18 VBUS presence and GPIO46 CST816D interrupt signaling are hardware-verified. Light sleep rebuilds GPIO18 HIGH-level USB insertion wake and GPIO46 LOW-level touch wake together before every cycle. Three consecutive GPIO18-only USB wake cycles, three consecutive GPIO46 touch wake cycles, and mixed touch/USB cycles passed on hardware. No CST816D register changes were required; wake is any TP_INT activity, not double-tap recognition.

The original repeated-cycle failure was caused by `Serial.flush()` after the backlight was turned off but before `esp_light_sleep_start()`. With native USB absent, the flush could block indefinitely, so the screen appeared asleep although the CPU had not entered light sleep and no configured wake source could act. A temporary independent 30-second timer test proved that removing the flush allowed entry and synchronous wake restoration. Do not reintroduce a blocking USB serial flush in this path.

WiFi normally reconnects 2-5 seconds after wake. Repeated testing exposed a separate recovery issue: a transient post-wake STA failure used the normal startup fallback and changed the in-memory configured mode to AP-only. Power-resume reconnects now preserve STA/APSTA mode and retry instead. During reconnection, the existing UI may briefly show its WiFi-disconnected screen before Moonraker state returns.

## IMU and orientation roadmap

**STATUS: PLANNED**

Waveshare reference code identifies a QMI8658 at I2C `0x6B`, provides three acceleration and three gyro axes, and computes acceleration-derived angles. Production firmware has no QMI8658 driver, task, state, or orientation logic. The existing LIS2DW12 code is for another target and is compiled out here.

Future orientation work must investigate driver integration on the shared I2C bus, mounting orientation, filtering and hysteresis, LVGL display rotation, TFT rotation, touch transforms, gesture directions, StatusBar placement, and application geometry. The logical relationship of the 240x240 KNOMI region and 240x80 StatusBar must remain coherent relative to the physical display. `tft.setRotation()` alone is not a complete solution.

## Generated-code boundaries

### Avoid manual editing

- `src/ui/**`: SquareLine Studio 1.3.1 output targeting LVGL 8.3.6.
- `_waveshare_demo/**`: Waveshare reference projects and bundled third-party code.
- `.pio/**`: generated build/dependency tree.
- `src/gif/*.c`, generated image/font arrays, and similar compiled assets.
- `lv_disp_(bugfix_backup).c`: a tracked reference/backup, not normal application code.

### Preferred integration areas

- `src/layout/**`
- `src/ui_overlay/**`
- `src/boards/**`
- `src/display/**`
- `src/lvgl_hal.cpp`
- `src/lvgl_usr.cpp`
- `src/moonraker.*`
- Active board pinout headers
- WiFi/web-server runtime modules when directly relevant

## Reproducibility / LVGL dependency

**STATUS: UNRESOLVED DISCREPANCY**

`platformio.ini` pins the LVGL dependency to the upstream v8.3.7 zip, but comments require applying LVGL pull request 4487 and adding `d->prev_scr = NULL;` near the immediate screen-load path in `lv_disp.c`. The repository also tracks `lv_disp_(bugfix_backup).c`.

Read-only comparison found:

- All four installed `.pio/libdeps/<environment>/lvgl/src/core/lv_disp.c` copies have the same SHA-256: `A95DF1C394479EE9F43AF41303C01D95D8E232D3BAACFB4512A367DBC65D5897`.
- That hash exactly matches the official upstream LVGL v8.3.7 `src/core/lv_disp.c` downloaded for comparison.
- Therefore the current installed dependencies are pristine, not manually patched.
- The tracked backup differs from upstream only in `lv_scr_load_anim()` by preventing a load of the active/already-pending screen, simplifying handling of an in-progress load, and clearing both `scr_to_load` and `prev_scr` after an immediate load.

The apparent intent is to prevent stale/pending screen state and repeated screen-load transitions from leaving invalid `prev_scr` ownership or animation state. The repository comment and backup preserve the expected change, but the dependency declaration does not apply it.

Risk:

- A clean install reproduces pristine 8.3.7, not the documented patched version.
- On this machine `.pio` is already pristine, so deleting it would not remove a currently present manual patch; nevertheless, reinstalling cannot resolve or reproduce the intended behavior.
- Another development machine will also obtain pristine upstream code unless the patch is applied separately.
- It is currently unclear whether recent application/layout changes made the patch unnecessary or whether a screen-transition bug remains latent.

Until resolved, do not casually delete/reinstall `.pio`, change LVGL versions, or assume the dependency comment has been satisfied. Do not hand-patch `.pio` as a durable fix. A later task should establish whether the patch is still required, then encode the decision reproducibly (for example through a tracked patch/build step or a dependency version containing the fix).

## Current known issues

- **WORKING / HARDWARE VERIFIED:** Layer indicator reads Moonraker `print_stats.info` when OrcaSlicer emits the required `SET_PRINT_STATS_INFO` commands; otherwise `--/--` is expected.
- **PARTIAL:** Battery percentage works, but calibration, charging and source detection are incomplete.
- **IMPLEMENTED / USB VERIFIED:** USB-aware power policy inhibits light sleep while GPIO18 is HIGH.
- **HARDWARE VERIFIED:** Battery inactivity light sleep after 60 seconds. Printer state is not consulted by the power policy; live active-print sleep remains a separate untested scenario.
- **HARDWARE VERIFIED:** Repeated and mixed GPIO46 LOW-level touch wake and GPIO18 HIGH-level USB-insertion wake; double-tap distinction is not implemented.
- **NOT IMPLEMENTED:** QMI8658 production driver.
- **NOT IMPLEMENTED:** Automatic display/input orientation.
- **PARTIAL:** Backlight API accepts levels 0-16, but this board currently drives GPIO 1 as binary on/off.
- **DISCREPANCY:** Board metadata/stub reports no battery while StatusBar runtime reads the board ADC directly.
- **CONCURRENCY RISK:** StatusBar reads Moonraker state across tasks without a synchronized snapshot.
- **REPRODUCIBILITY RISK:** Expected LVGL transition changes are documented but not applied by the dependency definition or current `.pio` tree.
- **ARCHITECTURAL ODDITY:** Per-screen bottom containers exist, but the persistent StatusBar correctly uses the global top layer instead.

## Current development priorities

1. Improve battery measurement behavior if evidence shows it is necessary.
2. Optionally validate the printer-state-independent sleep policy during a live print.
3. Integrate QMI8658.
4. Design automatic display and input orientation.

## Architectural constraints

- Preserve generated KNOMI navigation and `lv_scr_load_anim()` behavior.
- Keep the StatusBar fixed during normal screen transitions.
- Preserve the logical 240x240 application plus 240x80 StatusBar layout.
- Preserve the known-good BGR display configuration.
- Do not casually edit generated UI.
- Keep StatusBar rendering passive; acquire/interpret data elsewhere.
- Require evidence for hardware-specific assumptions.
- Preserve all existing uncommitted work.
- Do not prematurely generalize primary-board functionality.
- Treat builds as compile validation, not proof of hardware/runtime behavior.
