#pragma once

#include <cinttypes>

namespace hardware
{
    class battery
    {
    public:
        static battery &get()
        {
            if (sp_instance)
                return *sp_instance;

            sp_instance = new battery();

            return *sp_instance;
        };

        ~battery();

        battery(const battery &) = delete;
        battery(battery &&) = delete;
        battery &operator=(const battery &) = delete;
        battery &operator=(battery &&) = delete;

        uint32_t voltage_level();

    private:
        static battery *sp_instance;

        battery();

        void *mp_implementation = nullptr;
    };
}