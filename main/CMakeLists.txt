idf_component_register(
    SRCS 
        "shared_data.c" 
        "main.c"
        "sensor_task.c"
        "display_task.c"
        "wifi_manager.c"
    INCLUDE_DIRS "."
    REQUIRES
        htu21d
        esp_driver_i2c
        nvs_flash
        fatfs
        spi_flash
        esp_timer
        esp_wifi       
                    )
