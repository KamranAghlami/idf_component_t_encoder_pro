#include "hardware/storage.h"

#include <map>
#include <string>

#include <esp_littlefs.h>
#include <nvs_flash.h>

namespace hardware
{
    namespace storage
    {
        constexpr const char *PARTITION_LABEL = "storage";

        void mount(const type storage_type)
        {
            if (storage_type != type::nvs)
                return;

            esp_err_t err = nvs_flash_init();

            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
            {
                ESP_ERROR_CHECK(nvs_flash_erase());

                err = nvs_flash_init();
            }

            ESP_ERROR_CHECK(err);
        }

        void mount(const type storage_type, const char *mount_point)
        {
            if (storage_type != type::internal || esp_littlefs_mounted(PARTITION_LABEL))
                return;

            const esp_vfs_littlefs_conf_t littlefs_config = {
                .base_path = mount_point,
                .partition_label = PARTITION_LABEL,
                .partition = nullptr,
                .format_if_mount_failed = true,
                .read_only = false,
                .dont_mount = false,
                .grow_on_mount = true,
            };

            ESP_ERROR_CHECK(esp_vfs_littlefs_register(&littlefs_config));
        }

        void unmount(const type storage_type)
        {
            if (storage_type == type::nvs)
                ESP_ERROR_CHECK(nvs_flash_deinit());
            else if (storage_type == type::internal)
            {
                if (!esp_littlefs_mounted(PARTITION_LABEL))
                    return;

                ESP_ERROR_CHECK(esp_vfs_littlefs_unregister(PARTITION_LABEL));
            }
        }
    }
}