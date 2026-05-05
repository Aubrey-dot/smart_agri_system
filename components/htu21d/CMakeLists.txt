idf_component_register(
    SRCS "htu21d.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c
)