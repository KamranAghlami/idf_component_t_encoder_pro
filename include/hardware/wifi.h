#pragma once

#include <memory>

namespace hardware
{
    struct wifi_implementation;

    class wifi
    {
    public:
        enum class mode : uint8_t
        {
            ACCESS_POINT,
            STATION,
        };

        static wifi &get()
        {
            if (sp_instance)
                return *sp_instance;

            sp_instance = new wifi();

            return *sp_instance;
        };

        ~wifi();

        wifi(const wifi &) = delete;
        wifi(wifi &&) = delete;
        wifi &operator=(const wifi &) = delete;
        wifi &operator=(wifi &&) = delete;

        void set_mode(mode m);
        mode get_mode();

        void set_ssid(const char *ssid);
        const char *get_ssid();

        void set_password(const char *password);
        const char *get_password();

        const char *get_ip();
        const char *get_netmask();
        const char *get_gateway();

        void poll();
        void restart();

    private:
        static wifi *sp_instance;

        wifi();

        void start();
        void stop();

        std::unique_ptr<wifi_implementation> mp_implementation;
    };
}