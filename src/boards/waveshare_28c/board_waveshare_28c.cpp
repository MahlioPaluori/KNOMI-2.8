#include "boards/waveshare_28c/board_waveshare_28c.h"
#include "pinout.h"

#ifdef WAVESHARE28C

#include <driver/gpio.h>
#include <driver/i2c.h>

namespace {
static constexpr uint8_t kTca9554Address = 0x20;
static constexpr uint8_t kTca9554OutputReg = 0x01;
static constexpr uint8_t kTca9554ConfigReg = 0x03;
static constexpr uint8_t kTca9554ExioPin2 = 2;
static constexpr uint8_t kGt911Address = 0x5D;
static constexpr uint16_t kGt911StatusReg = 0x814E;
static constexpr uint16_t kGt911FirstPointReg = 0x814F;
static constexpr uint8_t kGt911StatusReady = 0x80;
static constexpr uint8_t kGt911TouchCountMask = 0x0F;
static constexpr int kTouchInterruptPin = 16;
static constexpr int kI2cMasterSda = 15;
static constexpr int kI2cMasterScl = 7;
static constexpr int kI2cMasterNum = 0;
static constexpr int kI2cMasterFreqHz = 400000;
static constexpr int kI2cMasterTimeoutMs = 1000;

uint8_t tca9554_read_reg(uint8_t reg) {
    uint8_t value = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return 0;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (kTca9554Address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (kTca9554Address << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    (void)i2c_master_cmd_begin(kI2cMasterNum, cmd, pdMS_TO_TICKS(kI2cMasterTimeoutMs));
    i2c_cmd_link_delete(cmd);
    return value;
}

void tca9554_write_reg(uint8_t reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (kTca9554Address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    (void)i2c_master_cmd_begin(kI2cMasterNum, cmd, pdMS_TO_TICKS(kI2cMasterTimeoutMs));
    i2c_cmd_link_delete(cmd);
}

void tca9554_set_pin(uint8_t pin, uint8_t state) {
    uint8_t output = tca9554_read_reg(kTca9554OutputReg);
    if (state == 1) {
        output = static_cast<uint8_t>(output | (1U << (pin - 1)));
    } else {
        output = static_cast<uint8_t>(output & ~(1U << (pin - 1)));
    }
    tca9554_write_reg(kTca9554OutputReg, output);
}

bool i2c_read(uint8_t address, uint16_t reg, uint8_t *data, size_t length) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return false;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, static_cast<uint8_t>(reg >> 8), true);
    i2c_master_write_byte(cmd, static_cast<uint8_t>(reg), true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true);
    if (length > 1) {
        i2c_master_read(cmd, data, length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(kI2cMasterNum, cmd, pdMS_TO_TICKS(kI2cMasterTimeoutMs));
    i2c_cmd_link_delete(cmd);
    return err == ESP_OK;
}

bool i2c_write(uint8_t address, uint16_t reg, const uint8_t *data, size_t length) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return false;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, static_cast<uint8_t>(reg >> 8), true);
    i2c_master_write_byte(cmd, static_cast<uint8_t>(reg), true);
    i2c_master_write(cmd, const_cast<uint8_t *>(data), length, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(kI2cMasterNum, cmd, pdMS_TO_TICKS(kI2cMasterTimeoutMs));
    i2c_cmd_link_delete(cmd);
    return err == ESP_OK;
}
}

void board_waveshare_28c_init(void) {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = kI2cMasterSda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = kI2cMasterScl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = kI2cMasterFreqHz;
    (void)i2c_param_config(kI2cMasterNum, &conf);
    (void)i2c_driver_install(kI2cMasterNum, conf.mode, 0, 0, 0);

    tca9554_write_reg(kTca9554ConfigReg, 0x00);
}

void board_waveshare_28c_display_init(void) {
}

void board_waveshare_28c_touch_init(void) {
    tca9554_set_pin(kTca9554ExioPin2, 0);
    pinMode(kTouchInterruptPin, OUTPUT);
    digitalWrite(kTouchInterruptPin, LOW);
    vTaskDelay(pdMS_TO_TICKS(10));
    tca9554_set_pin(kTca9554ExioPin2, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    pinMode(kTouchInterruptPin, INPUT);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void board_waveshare_28c_backlight_init(void) {
}

void board_waveshare_28c_backlight_set(int8_t level) {
    (void)level;
}

bool board_waveshare_28c_touch_read(uint16_t *x, uint16_t *y) {
    uint8_t status = 0;
    if (!i2c_read(kGt911Address, kGt911StatusReg, &status, sizeof(status))) {
        return false;
    }

    if ((status & kGt911StatusReady) == 0 || (status & kGt911TouchCountMask) == 0) {
        return false;
    }

    uint8_t point[8] = {};
    bool read_ok = i2c_read(kGt911Address, kGt911FirstPointReg, point, sizeof(point));
    uint8_t clear_status = 0;
    bool clear_ok = i2c_write(kGt911Address, kGt911StatusReg, &clear_status, sizeof(clear_status));
    if (!read_ok || !clear_ok) {
        return false;
    }

    if (x) {
        *x = static_cast<uint16_t>(point[1] | (point[2] << 8));
    }
    if (y) {
        *y = static_cast<uint16_t>(point[3] | (point[4] << 8));
    }
    return true;
}

board_info_t board_waveshare_28c_get_info(void) {
    board_info_t info = {"Waveshare 2.8C", TFT_WIDTH, TFT_HEIGHT, true, false, LayoutType::Square240};
    return info;
}

int16_t board_waveshare_28c_battery_percent(void) {
    return -1;
}

#endif
