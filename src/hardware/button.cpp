#include "hardware/button.h"

#include <algorithm>
#include <esp_timer.h>

namespace hardware
{
    std::vector<std::unique_ptr<button>> button::s_buttons;
    std::vector<button::key_event> button::s_key_events;

    void button::add(gpio_num_t pin, uint32_t id)
    {
        for (auto &btn : s_buttons)
            if (btn->m_pin == pin)
            {
                btn->m_id = id;

                return;
            }

        s_buttons.emplace_back(new button(pin, id));
    }

    void button::remove(gpio_num_t pin)
    {
        auto predicate = [&pin](const std::unique_ptr<button> &btn)
        {
            return btn->m_pin == pin;
        };

        s_buttons.erase(std::remove_if(s_buttons.begin(), s_buttons.end(), predicate), s_buttons.end());
    }

    void button::tick()
    {
        for (auto &btn : s_buttons)
            if (btn->m_last_state == gpio_get_level(btn->m_pin))
            {
                btn->m_last_state = !btn->m_last_state;

                s_key_events.push_back({btn->m_id, btn->m_last_state, (esp_timer_get_time() / 1000LL)});
            }
    }

    const button::key_event button::get_data()
    {
        button::tick();

        key_event data{.id = 0, .state = false, .timestamp = 0};

        if (s_key_events.size() && ((esp_timer_get_time() / 1000LL) - s_key_events.front().timestamp > 100))
        {
            auto same_state_it = s_key_events.begin() + 1;
            data = s_key_events.front();

            while (same_state_it != s_key_events.end() && same_state_it->state == data.state)
                data.id |= same_state_it++->id;

            s_key_events.erase(s_key_events.begin(), same_state_it);
        }

        return data;
    }

    button::button(gpio_num_t pin, uint32_t id) : m_pin(pin),
                                                  m_id(id),
                                                  m_last_state(false)
    {
        const gpio_config_t gpio_cfg = {
            .pin_bit_mask = (1ULL << pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        ESP_ERROR_CHECK(gpio_config(&gpio_cfg));
    }

    button::~button()
    {
        ESP_ERROR_CHECK(gpio_reset_pin(m_pin));
    }
}