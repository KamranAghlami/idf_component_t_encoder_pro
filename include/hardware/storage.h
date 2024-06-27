#pragma once

namespace hardware
{
    namespace storage
    {
        enum class type
        {
            nvs,
            internal,
        };

        void mount(const type storage_type);
        void mount(const type storage_type, const char *mount_point);
        void unmount(const type storage_type);
    }
}