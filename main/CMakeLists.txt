idf_component_register(
    SRCS 
        "shared_data.c" 
        "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        htu21d
        esp_driver_i2c
        nvs_flash
        fatfs
        spi_flash       
                    )
