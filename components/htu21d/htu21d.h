#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

//----I2C Address------------------------------------
#define HTU21D_I2C_ADDR 0X40

//--------Commands-----------------------------------------
#define HTU21D_CMD_TEMP 0xF3 //start temp measurement no hold master mode
#define HTU21D_CMD_HUM 0xF5 //start humidity measurement no-hold master mode
#define HTU21D_CMD_RESET 0xFE //reset to default state

//--------------------TIMING (ms)----------------------------------
#define HTU21D_RESET_DELAY_MS   15 
#define HTU21D_TEMP_DELAY_MS    50  //44ms max + margin
#define HTU21D_HUM_DELAY_MS     20 //14 ms max + margin

//----------------------ERROR-----------------------------------
#define HTU21D_ERR_CRC  (ESP_ERR_INVALID_CRC)

//----------------Return type-------------------------------------
typedef struct {
    float temperature;
    float humidity;
} htu21d_data_t;

//----------------Device Handle------------------------------------
typedef struct{
    i2c_master_dev_handle_t dev_handle;
} htu21d_handle_t;

//------------------------------------API--------------------------------------
esp_err_t htu21d_init(i2c_master_bus_handle_t bus, htu21d_handle_t *out_handle);
esp_err_t htu21d_read_temperature(htu21d_handle_t *handle, float *temperature);
esp_err_t htu21d_read_humidity(htu21d_handle_t *handle, float *humidity);
esp_err_t htu21d_read_all(htu21d_handle_t *handle, htu21d_data_t *data);

