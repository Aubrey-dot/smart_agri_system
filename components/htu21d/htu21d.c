#include "htu21d.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HTU21D";

//----Init---------------------------------------------------------------------------------
esp_err_t htu21d_init(i2c_master_bus_handle_t bus, htu21d_handle_t *out_handle){
    if (out_handle == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    //config htu21d on bus
    i2c_device_config_t conf={
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = HTU21D_I2C_ADDR,
        .scl_speed_hz = 400000,
    };

    //add device to bus
    esp_err_t ret = i2c_master_bus_add_device(bus, &conf,&out_handle->dev_handle);
    if (ret != ESP_OK){
        ESP_LOGE(TAG,"Failed to add device to bus: %s", esp_err_to_name(ret));
        return ret;
    }

    //reset HTU21D
    uint8_t cmd = HTU21D_CMD_RESET;
    ret = i2c_master_transmit(out_handle->dev_handle, &cmd, 1,pdMS_TO_TICKS(100));
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "Soft Reset Failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //wait for reset to complete
    vTaskDelay(pdMS_TO_TICKS(HTU21D_RESET_DELAY_MS));

    ESP_LOGI(TAG, "Initialized successfuly");
    return ESP_OK;
}

//----crc check------------------------------------------------------------------------
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

//----Read Temperature----------------------------------------------------------
esp_err_t htu21d_read_temperature(htu21d_handle_t *handle, float *temperature){
    if (handle == NULL || temperature == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;
    uint8_t cmd = HTU21D_CMD_TEMP;

    //send temp command
    ret = i2c_master_transmit(handle->dev_handle,&cmd, 1,pdMS_TO_TICKS(100));

    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Temp commad failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //wait for 50ms measurement
    vTaskDelay(pdMS_TO_TICKS(HTU21D_TEMP_DELAY_MS));

    //Read bytes: MSB, LSB, CRC
    uint8_t data[3];
    ret = i2c_master_receive(handle->dev_handle, data, 3,pdMS_TO_TICKS(100));
     if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read temp: %s", esp_err_to_name(ret));
        return ret;
    }

    //verify crc
    if (htu21d_crc_check(data,2) != data[2]){
        ESP_LOGE(TAG, "Temp CRC check failed");
        return HTU21D_ERR_CRC;
    }

    //combine bytes
    uint16_t raw = (data[0] << 8) | data[1];
    raw &= 0xFFFC; //remove status bits

    *temperature = -46.85 + (175.72 * raw /65536.0);

    return ESP_OK;
}

//-----Read Humidity----------------------------------------------------------------
esp_err_t htu21d_read_humidity(htu21d_handle_t *handle, float *humidity){
    if (handle == NULL || humidity == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;
    uint8_t cmd = HTU21D_CMD_HUM;

    //send humidity command
    ret = i2c_master_transmit(handle->dev_handle, &cmd, 1,pdMS_TO_TICKS(100));

    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Humidity commad failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //wait for 50ms measurement
    vTaskDelay(pdMS_TO_TICKS(HTU21D_HUM_DELAY_MS));

    //Read bytes: MSB, LSB, CRC
    uint8_t data[3];
    ret = i2c_master_receive(handle->dev_handle, data, 3,pdMS_TO_TICKS(100));
 
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read Humidity: %s", esp_err_to_name(ret));
        return ret;
    }

    //verify crc
    if (htu21d_crc_check(data,2) != data[2]){
        ESP_LOGE(TAG, "Humidity CRC check failed");
        return HTU21D_ERR_CRC;
    }

    //combine bytes
    uint16_t raw = (data[0] << 8) | data[1];
    raw &= 0xFFFC; //remove status bits

    *humidity = -6.0 + (125.0 * raw /65536.0);

    return ESP_OK;
}

//----Read All------------------------------------------------------------------------
esp_err_t htu21d_read_all(htu21d_handle_t *handle, htu21d_data_t *data){
    if (handle == NULL || data == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    ret =  htu21d_read_temperature(handle,&data->temperature);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read tempeature: %s",esp_err_to_name(ret));
        return ret;
    }

    ret = htu21d_read_humidity(handle, &data->humidity);
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to read Humidity: %s",esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}


