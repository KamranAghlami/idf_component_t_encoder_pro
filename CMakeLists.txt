file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")

idf_component_register(SRCS ${SOURCES} INCLUDE_DIRS "include" PRIV_INCLUDE_DIRS "src" PRIV_REQUIRES driver esp_timer nvs_flash esp_lcd esp_adc esp_wifi)
