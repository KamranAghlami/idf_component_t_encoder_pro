#include "hardware/battery.h"

#include <esp_adc/adc_oneshot.h>

constexpr adc_channel_t CHANNEL_BATTERY = ADC_CHANNEL_3;

namespace hardware
{
    struct battery_implementation
    {
        adc_oneshot_unit_handle_t adc_handle = nullptr;
        adc_cali_handle_t calibration_handle = nullptr;
    };

    battery *battery::sp_instance = nullptr;

    battery::battery() : mp_implementation(static_cast<void *>(new battery_implementation()))
    {
        auto implementation = static_cast<battery_implementation *>(mp_implementation);

        const adc_oneshot_unit_init_cfg_t adc_config = {
            .unit_id = ADC_UNIT_1,
            .clk_src = static_cast<adc_oneshot_clk_src_t>(0),
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };

        ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &implementation->adc_handle));

        const adc_oneshot_chan_cfg_t channel_config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ESP_ERROR_CHECK(adc_oneshot_config_channel(implementation->adc_handle, CHANNEL_BATTERY, &channel_config));

        const adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = adc_config.unit_id,
            .chan = CHANNEL_BATTERY,
            .atten = channel_config.atten,
            .bitwidth = channel_config.bitwidth,
        };

        ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &implementation->calibration_handle));
    }

    battery::~battery()
    {
        auto implementation = static_cast<battery_implementation *>(mp_implementation);

        ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(implementation->calibration_handle));
        ESP_ERROR_CHECK(adc_oneshot_del_unit(implementation->adc_handle));

        delete implementation;
    }

    uint32_t battery::voltage_level()
    {
        auto implementation = static_cast<battery_implementation *>(mp_implementation);

        int adc_raw = 0;
        int voltage = 0;

        ESP_ERROR_CHECK(adc_oneshot_read(implementation->adc_handle, CHANNEL_BATTERY, &adc_raw));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(implementation->calibration_handle, adc_raw, &voltage));

        return voltage * 2;
    }
}
