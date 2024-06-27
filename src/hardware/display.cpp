#include "hardware/display.h"

#include <driver/gpio.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>

constexpr gpio_num_t PIN_LCD_BACKLIGHT = GPIO_NUM_38;
constexpr gpio_num_t PIN_LCD_CS = GPIO_NUM_6;
constexpr gpio_num_t PIN_LCD_D0 = GPIO_NUM_39;
constexpr gpio_num_t PIN_LCD_D1 = GPIO_NUM_40;
constexpr gpio_num_t PIN_LCD_D2 = GPIO_NUM_41;
constexpr gpio_num_t PIN_LCD_D3 = GPIO_NUM_42;
constexpr gpio_num_t PIN_LCD_D4 = GPIO_NUM_45;
constexpr gpio_num_t PIN_LCD_D5 = GPIO_NUM_46;
constexpr gpio_num_t PIN_LCD_D6 = GPIO_NUM_47;
constexpr gpio_num_t PIN_LCD_D7 = GPIO_NUM_48;
constexpr gpio_num_t PIN_LCD_DC = GPIO_NUM_7;
constexpr gpio_num_t PIN_LCD_POWER = GPIO_NUM_15;
constexpr gpio_num_t PIN_LCD_RD = GPIO_NUM_9;
constexpr gpio_num_t PIN_LCD_RES = GPIO_NUM_5;
constexpr gpio_num_t PIN_LCD_WR = GPIO_NUM_8;

constexpr uint16_t LCD_PIXELS_WIDTH = 320;
constexpr uint16_t LCD_PIXELS_HEIGHT = 170;
constexpr uint8_t LCD_COLOR_SIZE = 2;

namespace hardware
{
    struct display_implementation
    {
        esp_lcd_i80_bus_handle_t bus_handle = nullptr;
        esp_lcd_panel_io_handle_t io_handle = nullptr;
        esp_lcd_panel_handle_t panel_handle = nullptr;
    };

    display *display::sp_instance = nullptr;

    display::display() : mp_implementation(std::make_unique<display_implementation>())
    {
        const gpio_config_t gpio_cfg = {
            .pin_bit_mask = (1ULL << PIN_LCD_RD) | (1ULL << PIN_LCD_POWER) | (1ULL << PIN_LCD_BACKLIGHT),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
        ESP_ERROR_CHECK(gpio_set_level(PIN_LCD_RD, 1));
        ESP_ERROR_CHECK(gpio_set_level(PIN_LCD_POWER, 1));

        set_backlight(brightness_level::min);

        const esp_lcd_i80_bus_config_t bus_config = {
            .dc_gpio_num = PIN_LCD_DC,
            .wr_gpio_num = PIN_LCD_WR,
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .data_gpio_nums =
                {
                    PIN_LCD_D0,
                    PIN_LCD_D1,
                    PIN_LCD_D2,
                    PIN_LCD_D3,
                    PIN_LCD_D4,
                    PIN_LCD_D5,
                    PIN_LCD_D6,
                    PIN_LCD_D7,
                },
            .bus_width = 8,
            .max_transfer_bytes = LCD_COLOR_SIZE * LCD_PIXELS_WIDTH * 16,
            .psram_trans_align = 32,
            .sram_trans_align = 4,
        };

        ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &(mp_implementation->bus_handle)));

        auto on_transfer_done = [](esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
        {
            auto disp = static_cast<display *>(user_ctx);

            disp->m_on_transfer_done_callback(disp->m_on_transfer_done_user_data);

            return false;
        };

        const esp_lcd_panel_io_i80_config_t io_config = {
            .cs_gpio_num = PIN_LCD_CS,
            .pclk_hz = 10 * 1000 * 1000,
            .trans_queue_depth = 20,
            .on_color_trans_done = on_transfer_done,
            .user_ctx = this,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .dc_levels = {
                .dc_idle_level = 0,
                .dc_cmd_level = 0,
                .dc_dummy_level = 0,
                .dc_data_level = 1,
            },
            .flags = {
                .cs_active_high = 0,
                .reverse_color_bits = 0,
                .swap_color_bytes = 0,
                .pclk_active_neg = 0,
                .pclk_idle_low = 0,
            },
        };

        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(mp_implementation->bus_handle, &io_config, &(mp_implementation->io_handle)));

        const esp_lcd_panel_dev_config_t device_config = {
            .reset_gpio_num = PIN_LCD_RES,
            .rgb_endian = LCD_RGB_ENDIAN_RGB,
            .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
            .bits_per_pixel = 16,
            .flags = {
                .reset_active_high = 0,
            },
            .vendor_config = nullptr,
        };

        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(mp_implementation->io_handle, &device_config, &(mp_implementation->panel_handle)));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(mp_implementation->panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(mp_implementation->panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(mp_implementation->panel_handle, true));
        ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(mp_implementation->panel_handle, true));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(mp_implementation->panel_handle, false, true));
        ESP_ERROR_CHECK(esp_lcd_panel_set_gap(mp_implementation->panel_handle, 0, 35));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(mp_implementation->panel_handle, true));
    }

    display::~display()
    {
        ESP_ERROR_CHECK(gpio_reset_pin(PIN_LCD_BACKLIGHT));

        ESP_ERROR_CHECK(esp_lcd_panel_del(mp_implementation->panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_io_del(mp_implementation->io_handle));
        ESP_ERROR_CHECK(esp_lcd_del_i80_bus(mp_implementation->bus_handle));

        ESP_ERROR_CHECK(gpio_reset_pin(PIN_LCD_POWER));
        ESP_ERROR_CHECK(gpio_reset_pin(PIN_LCD_RD));
    }

    uint16_t display::width()
    {
        return LCD_PIXELS_WIDTH;
    }

    uint16_t display::height()
    {
        return LCD_PIXELS_HEIGHT;
    }

    void display::set_backlight(brightness_level level)
    {
        switch (level)
        {
        case brightness_level::min:
            ESP_ERROR_CHECK(gpio_set_level(PIN_LCD_BACKLIGHT, 0));
            break;

        case brightness_level::max:
            ESP_ERROR_CHECK(gpio_set_level(PIN_LCD_BACKLIGHT, 1));
            break;

        default:
            break;
        }
    }

    void display::set_transfer_done_callback(transfer_done_callback_t on_transfer_done, void *user_data)
    {
        m_on_transfer_done_callback = on_transfer_done;
        m_on_transfer_done_user_data = user_data;
    }

    void display::set_bitmap(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint16_t *data)
    {
        esp_lcd_panel_draw_bitmap(mp_implementation->panel_handle, x1, y1, x2 + 1, y2 + 1, data);
    }
}
