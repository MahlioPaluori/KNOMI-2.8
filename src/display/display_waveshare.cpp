#include "display_waveshare.h"
#include "pinout.h"
#include "boards/board_layer.h"

#ifdef WAVESHARE28C

#include <Arduino.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {

static constexpr uint16_t kWaveshareWidth = 480;
static constexpr uint16_t kWaveshareHeight = 480;

static constexpr uint8_t kTca9554Address = 0x20;
static constexpr uint8_t kTca9554InputReg = 0x00;
static constexpr uint8_t kTca9554OutputReg = 0x01;
static constexpr uint8_t kTca9554ConfigReg = 0x03;
static constexpr uint8_t kTca9554ExioPin1 = 1;
static constexpr uint8_t kTca9554ExioPin3 = 3;
static constexpr int kI2cMasterSda = 15;
static constexpr int kI2cMasterScl = 7;
static constexpr int kI2cMasterNum = 0;
static constexpr int kI2cMasterFreqHz = 400000;
static constexpr int kI2cMasterTimeoutMs = 1000;

spi_device_handle_t s_spi_handle = nullptr;
esp_lcd_panel_handle_t s_panel_handle = nullptr;

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
    esp_err_t err = i2c_master_cmd_begin(kI2cMasterNum, cmd, pdMS_TO_TICKS(kI2cMasterTimeoutMs));
    i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) {
        return 0;
    }
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
    esp_err_t err = i2c_master_cmd_begin(kI2cMasterNum, cmd, pdMS_TO_TICKS(kI2cMasterTimeoutMs));
    i2c_cmd_link_delete(cmd);
    (void)err;
}

void tca9554_init(void) {
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

void tca9554_set_pin(uint8_t pin, uint8_t state) {
    uint8_t output = tca9554_read_reg(kTca9554OutputReg);
    if (state == 1) {
        output = static_cast<uint8_t>(output | (1U << (pin - 1)));
    } else {
        output = static_cast<uint8_t>(output & ~(1U << (pin - 1)));
    }
    tca9554_write_reg(kTca9554OutputReg, output);
}

void st7701_write_command(uint8_t cmd) {
    spi_transaction_t spi_tran = {};
    spi_tran.cmd = 0;
    spi_tran.addr = cmd;
    spi_tran.length = 0;
    spi_tran.rxlength = 0;
    (void)spi_device_transmit(s_spi_handle, &spi_tran);
}

void st7701_write_data(uint8_t data) {
    spi_transaction_t spi_tran = {};
    spi_tran.cmd = 1;
    spi_tran.addr = data;
    spi_tran.length = 0;
    spi_tran.rxlength = 0;
    (void)spi_device_transmit(s_spi_handle, &spi_tran);
}

void st7701_cs_en(void) {
    tca9554_set_pin(kTca9554ExioPin3, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void st7701_cs_dis(void) {
    tca9554_set_pin(kTca9554ExioPin3, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void st7701_reset(void) {
    tca9554_set_pin(kTca9554ExioPin1, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    tca9554_set_pin(kTca9554ExioPin1, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void st7701_init_controller(void) {
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = 1;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = 2;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 64 * 1024;

    (void)spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {};
    devcfg.command_bits = 1;
    devcfg.address_bits = 8;
    devcfg.mode = SPI_MODE0;
    devcfg.clock_speed_hz = 40 * 1000 * 1000;
    devcfg.spics_io_num = -1;
    devcfg.queue_size = 1;

    (void)spi_bus_add_device(SPI2_HOST, &devcfg, &s_spi_handle);

    st7701_cs_en();

    st7701_write_command(0xFF);
    st7701_write_data(0x77);
    st7701_write_data(0x01);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x13);

    st7701_write_command(0xEF);
    st7701_write_data(0x08);

    st7701_write_command(0xFF);
    st7701_write_data(0x77);
    st7701_write_data(0x01);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x10);

    st7701_write_command(0xC0);
    st7701_write_data(0x3B);
    st7701_write_data(0x00);

    st7701_write_command(0xC1);
    st7701_write_data(0x10);
    st7701_write_data(0x0C);

    st7701_write_command(0xC2);
    st7701_write_data(0x07);
    st7701_write_data(0x0A);

    st7701_write_command(0xC7);
    st7701_write_data(0x00);

    st7701_write_command(0xCC);
    st7701_write_data(0x10);

    st7701_write_command(0xCD);
    st7701_write_data(0x08);

    st7701_write_command(0xB0);
    st7701_write_data(0x05);
    st7701_write_data(0x12);
    st7701_write_data(0x98);
    st7701_write_data(0x0E);
    st7701_write_data(0x0F);
    st7701_write_data(0x07);
    st7701_write_data(0x07);
    st7701_write_data(0x09);
    st7701_write_data(0x09);
    st7701_write_data(0x23);
    st7701_write_data(0x05);
    st7701_write_data(0x52);
    st7701_write_data(0x0F);
    st7701_write_data(0x67);
    st7701_write_data(0x2C);
    st7701_write_data(0x11);

    st7701_write_command(0xB1);
    st7701_write_data(0x0B);
    st7701_write_data(0x11);
    st7701_write_data(0x97);
    st7701_write_data(0x0C);
    st7701_write_data(0x12);
    st7701_write_data(0x06);
    st7701_write_data(0x06);
    st7701_write_data(0x08);
    st7701_write_data(0x08);
    st7701_write_data(0x22);
    st7701_write_data(0x03);
    st7701_write_data(0x51);
    st7701_write_data(0x11);
    st7701_write_data(0x66);
    st7701_write_data(0x2B);
    st7701_write_data(0x0F);

    st7701_write_command(0xFF);
    st7701_write_data(0x77);
    st7701_write_data(0x01);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x11);

    st7701_write_command(0xB0);
    st7701_write_data(0x5D);

    st7701_write_command(0xB1);
    st7701_write_data(0x3E);

    st7701_write_command(0xB2);
    st7701_write_data(0x81);

    st7701_write_command(0xB3);
    st7701_write_data(0x80);

    st7701_write_command(0xB5);
    st7701_write_data(0x4E);

    st7701_write_command(0xB7);
    st7701_write_data(0x85);

    st7701_write_command(0xB8);
    st7701_write_data(0x20);

    st7701_write_command(0xC1);
    st7701_write_data(0x78);

    st7701_write_command(0xC2);
    st7701_write_data(0x78);

    st7701_write_command(0xD0);
    st7701_write_data(0x88);

    st7701_write_command(0xE0);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x02);

    st7701_write_command(0xE1);
    st7701_write_data(0x06);
    st7701_write_data(0x30);
    st7701_write_data(0x08);
    st7701_write_data(0x30);
    st7701_write_data(0x05);
    st7701_write_data(0x30);
    st7701_write_data(0x07);
    st7701_write_data(0x30);
    st7701_write_data(0x00);
    st7701_write_data(0x33);
    st7701_write_data(0x33);

    st7701_write_command(0xE2);
    st7701_write_data(0x11);
    st7701_write_data(0x11);
    st7701_write_data(0x33);
    st7701_write_data(0x33);
    st7701_write_data(0xF4);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0xF4);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x00);

    st7701_write_command(0xE3);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x11);
    st7701_write_data(0x11);

    st7701_write_command(0xE4);
    st7701_write_data(0x44);
    st7701_write_data(0x44);

    st7701_write_command(0xE5);
    st7701_write_data(0x0D);
    st7701_write_data(0xF5);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);
    st7701_write_data(0x0F);
    st7701_write_data(0xF7);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);
    st7701_write_data(0x09);
    st7701_write_data(0xF1);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);
    st7701_write_data(0x0B);
    st7701_write_data(0xF3);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);

    st7701_write_command(0xE6);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x11);
    st7701_write_data(0x11);

    st7701_write_command(0xE7);
    st7701_write_data(0x44);
    st7701_write_data(0x44);

    st7701_write_command(0xE8);
    st7701_write_data(0x0C);
    st7701_write_data(0xF4);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);
    st7701_write_data(0x0E);
    st7701_write_data(0xF6);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);
    st7701_write_data(0x08);
    st7701_write_data(0xF0);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);
    st7701_write_data(0x0A);
    st7701_write_data(0xF2);
    st7701_write_data(0x30);
    st7701_write_data(0xF0);

    st7701_write_command(0xE9);
    st7701_write_data(0x36);
    st7701_write_data(0x01);

    st7701_write_command(0xEB);
    st7701_write_data(0x00);
    st7701_write_data(0x01);
    st7701_write_data(0xE4);
    st7701_write_data(0xE4);
    st7701_write_data(0x44);
    st7701_write_data(0x88);
    st7701_write_data(0x40);

    st7701_write_command(0xED);
    st7701_write_data(0xFF);
    st7701_write_data(0x10);
    st7701_write_data(0xAF);
    st7701_write_data(0x76);
    st7701_write_data(0x54);
    st7701_write_data(0x2B);
    st7701_write_data(0xCF);
    st7701_write_data(0xFF);
    st7701_write_data(0xFF);
    st7701_write_data(0xFC);
    st7701_write_data(0xB2);
    st7701_write_data(0x45);
    st7701_write_data(0x67);
    st7701_write_data(0xFA);
    st7701_write_data(0x01);
    st7701_write_data(0xFF);

    st7701_write_command(0xEF);
    st7701_write_data(0x08);
    st7701_write_data(0x08);
    st7701_write_data(0x08);
    st7701_write_data(0x45);
    st7701_write_data(0x3F);
    st7701_write_data(0x54);

    st7701_write_command(0xFF);
    st7701_write_data(0x77);
    st7701_write_data(0x01);
    st7701_write_data(0x00);
    st7701_write_data(0x00);
    st7701_write_data(0x00);

    st7701_write_command(0x11);
    delay(120);

    st7701_write_command(0x3A);
    st7701_write_data(0x66);

    st7701_write_command(0x36);
    st7701_write_data(0x00);

    st7701_write_command(0x35);
    st7701_write_data(0x00);

    st7701_write_command(0x29);

    st7701_cs_dis();
}

void waveshare_panel_init(void) {
    esp_lcd_rgb_panel_config_t panel_config = {};
    panel_config.clk_src = LCD_CLK_SRC_PLL240M;
    panel_config.timings.pclk_hz = 30 * 1000 * 1000;
    panel_config.timings.h_res = kWaveshareWidth;
    panel_config.timings.v_res = kWaveshareHeight;
    panel_config.timings.hsync_pulse_width = 8;
    panel_config.timings.hsync_back_porch = 10;
    panel_config.timings.hsync_front_porch = 50;
    panel_config.timings.vsync_pulse_width = 2;
    panel_config.timings.vsync_back_porch = 18;
    panel_config.timings.vsync_front_porch = 8;
    panel_config.timings.flags.pclk_active_neg = 0;
    panel_config.data_width = 16;
    panel_config.sram_trans_align = 4;
    panel_config.psram_trans_align = 64;
    panel_config.hsync_gpio_num = 38;
    panel_config.vsync_gpio_num = 39;
    panel_config.de_gpio_num = 40;
    panel_config.pclk_gpio_num = 41;
    panel_config.disp_gpio_num = -1;
    panel_config.data_gpio_nums[0] = 5;
    panel_config.data_gpio_nums[1] = 45;
    panel_config.data_gpio_nums[2] = 48;
    panel_config.data_gpio_nums[3] = 47;
    panel_config.data_gpio_nums[4] = 21;
    panel_config.data_gpio_nums[5] = 14;
    panel_config.data_gpio_nums[6] = 13;
    panel_config.data_gpio_nums[7] = 12;
    panel_config.data_gpio_nums[8] = 11;
    panel_config.data_gpio_nums[9] = 10;
    panel_config.data_gpio_nums[10] = 9;
    panel_config.data_gpio_nums[11] = 46;
    panel_config.data_gpio_nums[12] = 3;
    panel_config.data_gpio_nums[13] = 8;
    panel_config.data_gpio_nums[14] = 18;
    panel_config.data_gpio_nums[15] = 17;
    panel_config.flags.fb_in_psram = true;

    esp_lcd_panel_handle_t panel = nullptr;
    esp_err_t err = esp_lcd_new_rgb_panel(&panel_config, &panel);
    if (err != ESP_OK) {
        return;
    }

    s_panel_handle = panel;
    (void)esp_lcd_panel_reset(panel);
    (void)esp_lcd_panel_init(panel);
    (void)esp_lcd_panel_disp_on_off(panel, true);
}

} // namespace

void display_waveshare_init(void) {
    tca9554_init();
    board_touch_init();
    st7701_reset();
    st7701_init_controller();
    waveshare_panel_init();
}

void display_waveshare_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    if (!s_panel_handle) {
        lv_disp_flush_ready(disp);
        return;
    }

    uint16_t x1 = static_cast<uint16_t>(area->x1);
    uint16_t y1 = static_cast<uint16_t>(area->y1);
    uint16_t x2 = static_cast<uint16_t>(area->x2 + 1);
    uint16_t y2 = static_cast<uint16_t>(area->y2 + 1);

    if (x2 >= kWaveshareWidth) {
        x2 = kWaveshareWidth;
    }
    if (y2 >= kWaveshareHeight) {
        y2 = kWaveshareHeight;
    }

    esp_lcd_panel_draw_bitmap(s_panel_handle, x1, y1, x2, y2, color_p);
    lv_disp_flush_ready(disp);
}

void display_waveshare_set_brightness(int8_t level) {
    board_backlight_set(level);
}

bool display_waveshare_has_touch(void) {
    return true;
}

bool display_waveshare_read_touch(uint16_t *x, uint16_t *y) {
    return board_touch_read(x, y);
}

#endif // WAVESHARE28C
