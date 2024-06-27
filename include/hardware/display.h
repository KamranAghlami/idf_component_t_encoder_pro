#pragma once

#include <cstdint>
#include <memory>

namespace hardware
{
    struct display_implementation;

    class display
    {
    public:
        using transfer_done_callback_t = void (*)(void *);

        enum class brightness_level
        {
            min = 0,
            max = 0xff,
        };

        static display &get()
        {
            if (sp_instance)
                return *sp_instance;

            sp_instance = new display();

            return *sp_instance;
        };

        ~display();

        display(const display &) = delete;
        display(display &&) = delete;
        display &operator=(const display &) = delete;
        display &operator=(display &&) = delete;

        uint16_t width();
        uint16_t height();

        void set_backlight(brightness_level level);
        void set_transfer_done_callback(transfer_done_callback_t on_transfer_done, void *user_data);
        void set_bitmap(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint16_t *data);

    private:
        static display *sp_instance;

        display();

        std::unique_ptr<display_implementation> mp_implementation;

        transfer_done_callback_t m_on_transfer_done_callback = nullptr;
        void *m_on_transfer_done_user_data = nullptr;
    };
}