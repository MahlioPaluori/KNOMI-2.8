# KNOMI firmware agent guide

## Project scope

This is KNOMI-derived embedded firmware built with Arduino, PlatformIO, ESP32, LVGL, and Moonraker/Klipper integration. Current development prioritizes the Waveshare `ESP32-S3-Touch-LCD-2` through PlatformIO environment `esp32_s3_touch_lcd_2`. It adapts the original 240x240 KNOMI interface to a 240x320 board and adds persistent runtime UI and board-specific features.

Keep other targets working where reasonably possible, but do not introduce cross-board abstractions merely for architectural purity. This is not currently a general board-layer redesign.

## Working-tree safety and scope

Before editing, inspect `git status` and the relevant `git diff`. Important uncommitted user work may be present and may overlap the task.

Unless explicitly requested, never reset, revert, overwrite, delete, stash, clean, commit, or push user changes. Do not assume a modified file belongs to the current task. Change only necessary files; avoid opportunistic refactors, unrelated cleanup, and premature generalization.

## Investigate before implementing

For hardware-specific or architecture-sensitive work, investigate first. Do not guess GPIOs, polarity, ADC ratios, I2C addresses, wake/interrupt wiring, charger behavior, sensor mounting, display-controller behavior, touch capabilities, or LVGL object ownership.

Search in this order where applicable:

1. Active board implementation and pinout.
2. Existing production code.
3. Bundled Waveshare demos.
4. Local drivers/libraries.
5. Relevant upstream implementation.

Clearly label conclusions as **VERIFIED**, **INFERRED**, or **UNKNOWN**. Never turn an inference into an implementation fact.

## Primary board path

`ESP32_S3_TOUCH_LCD_2` selects:

- Pinout: `src/pinout_esp32_s3_touch_lcd_2.h`
- Board backend: `src/boards/esp32_s3_touch_lcd_2/`
- Board facade: `src/boards/board_layer.cpp`
- Display backend: `src/display/display_esp32_s3_touch_lcd_2.cpp`
- Layout: `src/layout/layout_manager.cpp`
- Touch backend: `board_esp32_s3_touch_lcd_2_touch_*`

Verified current characteristics are ESP32-S3, 240x320 ST7789, CST816D at I2C address `0x15`, shared I2C on SDA 48/SCL 47, and battery ADC on GPIO 5. Vendor references identify a QMI8658 on that bus, but production firmware does not integrate it.

## Build validation

For board-specific changes, build with:

```powershell
platformio run -e esp32_s3_touch_lcd_2
```

Use the repository's installed PlatformIO executable if `platformio` is not on `PATH`. Do not claim success unless the command finishes successfully. Old artifacts, compilation starting, or truncated output are not proof. Build unrelated environments only when shared changes reasonably put them at risk or the user requests it.

Report modified files, behavior changed, target built, and actual result.

Do not delete/reinstall `.pio`, upgrade LVGL, or replace its dependency casually. The repository documents an expected LVGL screen-transition patch, but the currently installed dependency is pristine 8.3.7; see `docs/PROJECT_CONTEXT.md`.

## Generated UI boundary

`src/ui/**` is SquareLine-generated. Avoid manual edits unless explicitly necessary and justified. Do not solve runtime layout issues by patching every generated screen.

Prefer integration in `src/layout/**`, `src/ui_overlay/**`, `src/boards/**`, `src/display/**`, `src/lvgl_hal.cpp`, `src/lvgl_usr.cpp`, `src/moonraker.*`, and board pinout headers. Treat `_waveshare_demo/**`, `.pio/**`, and compiled image/GIF arrays as vendor, generated, or dependency material.

## LVGL invariants

For the primary board, preserve this logical layout:

```text
240x320 physical display
|- top:    240x240 KNOMI application region
`- bottom: 240x80 persistent StatusBar
```

Generated KNOMI screens remain LVGL screens and navigate through `lv_scr_load_anim()`. `ui_root_layout` wraps their children into the application region. The StatusBar is parented to `lv_layer_top()`, so it remains stationary while normal screens animate.

Do not casually change generated screen ownership, screen dimensions, `ui_root_layout`, `lv_scr_load_anim()`, display dimensions, StatusBar parentage, or TopLayer usage. Investigate their interaction first.

When using `lv_obj_remove_style_all()`, remember that it removes style-backed size/alignment properties. Reset/neutralize styles first, then apply required geometry and styling deliberately. Reordering StatusBar initialization previously collapsed its intended 240x80 root to roughly 130x130.

## StatusBar contract

The StatusBar is a passive view fixed in the physical bottom region:

- Left: green `WiFi` when connected, red `WiFi` otherwise. Do not restore RSSI text unless requested.
- Center: `current/total`, such as `35/175`; hardware-verified when the slicer supplies layer counters.
- Right: battery icon and percentage.

Keep HTTP, WiFi polling, ADC sampling/conversion, and hardware polling out of `status_bar.cpp`. Runtime/data modules prepare display state and call setters.

Layer data flows through `MOONRAKER::get_progress()` into `moonraker.data`, then `status_bar_runtime_update()`. The working source is `print_stats.info.current_layer` and `total_layer`. When debugging missing layer information, first verify that generated G-code contains `SET_PRINT_STATS_INFO` for total and current layers before modifying the Moonraker parser or StatusBar. Without slicer-supplied counters, `--/--` is expected.

## Battery, touch, and power roadmap

Current board-specific battery handling samples GPIO 5 twelve times every five seconds, drops one minimum and maximum, averages ten calibrated-millivolt readings, multiplies by 3, maps voltage through a LiPo curve, and updates the StatusBar. It is voltage-derived, not a fuel gauge. GPIO18 VBUS presence is hardware-verified through an external divider, but calibration, charging state, and the board abstraction remain incomplete. Do not claim charging detection without a verified signal or generalize this into a universal power framework unless requested.

Board-specific light-sleep power management is **HARDWARE VERIFIED** for `esp32_s3_touch_lcd_2`: after 60 seconds without touch on battery, regardless of printer state, it suspends WiFi, turns off the backlight, preserves LVGL state, and enters light sleep. USB power inhibits sleep. GPIO18 HIGH (USB insertion) and GPIO46 LOW (CST816D TP_INT) are enabled together as level-triggered wake sources and have passed repeated and mixed wake cycles. Never call `Serial.flush()` in the battery sleep-entry path: native USB CDC can block without a host before `esp_light_sleep_start()`. Wake restores brightness/UI immediately and WiFi typically reconnects in 2-5 seconds. Post-wake STA failures must retry without changing the configured mode to AP-only. The backend remains polling while awake, double-tap distinction is not used, and TP_RESET is not MCU-controlled.

## IMU and orientation roadmap

Vendor references identify a six-axis QMI8658, but production firmware has no QMI8658 driver or task. Automatic orientation is **PLANNED**. Before implementing, analyze shared I2C, mounting orientation, filtering/hysteresis, LVGL rotation, TFT rotation, touch transforms, gestures, application geometry, and StatusBar placement. Changing `tft.setRotation()` alone is insufficient. Preserve the conceptual 240x240 application plus 240x80 StatusBar relationship.

## Display configuration

The primary board's known-good path is LVGL -> `usr_disp_flush()` -> display facade -> ESP32-S3-Touch-LCD-2 backend -> TFT_eSPI -> ST7789/SPI. Preserve 240x320, TFT rotation 0, RGB565 (`LV_COLOR_DEPTH=16`, `LV_COLOR_16_SWAP=0`), `TFT_BGR`, inversion on, `pushColors(..., true)` byte swapping, and 40 MHz SPI unless board-specific evidence supports a change. BGR is required for correct current colors.

## Runtime diagnostics and truthful reporting

For runtime/hardware bugs, prefer evidence over speculative rewrites. Temporary targeted serial logging is allowed when its purpose is explained. Keep it minimal, capture evidence, remove it afterward unless retention is requested, and avoid production log spam.

Documentation and reports must distinguish **IMPLEMENTED**, **PARTIAL**, **PLANNED**, and **UNKNOWN**. A successful build does not prove runtime behavior or resolve a hardware bug.
