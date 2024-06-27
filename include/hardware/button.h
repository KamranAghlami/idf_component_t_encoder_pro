#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include <driver/gpio.h>

namespace hardware
{
    class button
    {
    public:
        struct key_event
        {
            uint32_t id;
            bool state;
            int64_t timestamp;
        };

        static void add(gpio_num_t pin, uint32_t id);
        static void remove(gpio_num_t pin);
        static const key_event get_data();

        ~button();

    private:
        static std::vector<std::unique_ptr<button>> s_buttons;
        static std::vector<key_event> s_key_events;

        static void tick();

        button(gpio_num_t pin, uint32_t id);

        gpio_num_t m_pin;
        uint32_t m_id;

        bool m_last_state;
    };
}