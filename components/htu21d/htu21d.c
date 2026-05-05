#include "htu21d.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HTU21D";

esp_err_t htu21d_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl){
    i2c_config_t conf={
        .mode = I2C_MODE_MASTER,
        .sda_io_num =   sda,
        .scl_io_num =   scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    //apply config to i2c peripheral
    esp_err_t ret = i2c_param_config(port, &conf);
    if (ret != ESP_OK){
        ESP_LOGE(TAG,"I2C param config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //install driver
    ret = i2c_driver_install(port,I2C_MODE_MASTER, 0, 0,0);
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //reset HTU21D
    uint8_t cmd = HTU21D_CMD_RESET;
    ret = i2c_master_write_to_device(port, HTU21D_I2C_ADDR, &cmd, 1,pdMS_TO_TICKS(100));
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "Soft Reset Failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //wait for reset to complete
    vTaskDelay(pdMS_TO_TICKS(HTU21D_RESET_DELAY_MS));

    ESP_LOGI(TAG, "Initialized successfuly");
    return ESP_OK;
}

//crc check
static uint8_t htu21d_crc_check(uint8_t *data, uint8_t len){
    uint8_t crc = 0X00;

    for (uint8_t i = 0; i < len; i++){
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++){
            if (crc & 0X80){
                crc = (crc <<1) ^ 0x31;
            }
            else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

//read temperature
esp_err_t htu21d_read_temperature(i2c_port_t port, float *temperature){
    if (temperature == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;
    uint8_t cmd = HTU21D_CMD_TEMP;

    //send temp command
    ret = i2c_master_write_to_device(port, HTU21D_I2C_ADDR, &cmd, 1,pdMS_TO_TICKS(100));

    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Temp commad failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //wait for 50ms measurement
    vTaskDelay(pdMS_TO_TICKS(HTU21D_TEMP_DELAY_MS));

    //Read bytes: MSB, LSB, CRC
    uint8_t data[3];
    if (htu21d_crc_check(data,2) != data[2]){
        ESP_LOGE(TAG, "Temp CRC check failed");
        return HTU21D_ERR_CRC;
    }

    ret = i2c_master_read_from_device(port, HTU21D_I2C_ADDR, data, 3,pdMS_TO_TICKS(100));
 
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read temp: %s", esp_err_to_name(ret));
        return ret;
    }

    //combine bytes
    uint16_t raw = (data[0] << 8) | data[1];
    raw &= 0xFFFC; //remove status bits

    *temperature = -46.85 + (175.72 * raw /65536.0);

    return ESP_OK;
}

//read humidity
esp_err_t htu21d_read_humidity(i2c_port_t port, float *humidity){
    if (humidity == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;
    uint8_t cmd = HTU21D_CMD_HUM;

    //send humidity command
    ret = i2c_master_write_to_device(port, HTU21D_I2C_ADDR, &cmd, 1,pdMS_TO_TICKS(100));

    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Humidity commad failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //wait for 50ms measurement
    vTaskDelay(pdMS_TO_TICKS(HTU21D_HUM_DELAY_MS));

    //Read bytes: MSB, LSB, CRC
    uint8_t data[3];
    if (htu21d_crc_check(data,2) != data[2]){
        ESP_LOGE(TAG, "Humidity CRC check failed");
        return HTU21D_ERR_CRC;
    }
    
    ret = i2c_master_read_from_device(port, HTU21D_I2C_ADDR, data, 3,pdMS_TO_TICKS(100));
 
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read Humidity: %s", esp_err_to_name(ret));
        return ret;
    }

    //combine bytes
    uint16_t raw = (data[0] << 8) | data[1];
    raw &= 0xFFFC; //remove status bits

    *humidity = -6.0 + (125.0 * raw /65536.0);

    return ESP_OK;
}

esp_err_t htu21d_read_all(i2c_port_t port, htu21d_data_t *data){
    if (data == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    ret =  htu21d_read_temperature(port,&data->temperature);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read tempeature: %s",esp_err_to_name(ret));
        return ret;
    }

    ret = htu21d_read_humidity(port, &data->humidity);
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read Humidity: %s",esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}


