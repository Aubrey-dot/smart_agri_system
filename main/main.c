#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "htu21d.h"
#include "sensor_task.h"
#include "display_task.h"

static const char *TAG = "MAIN";

//----I2C Bus Config----------------------------------------
#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN GPIO_NUM_21
#define I2C_SCL_PIN GPIO_NUM_22
#define I2C_SPEED_HZ 400000

void app_main(void)
{
    //configure i2c bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg,&bus_handle));
    ESP_LOGI(TAG, "I2C bus created");
/*
    ESP_LOGI(TAG, "Scanning I2C bus...");   for (uint8_t addr = 1; addr < 127; addr++) {
        esp_err_t ret = i2c_master_probe(bus_handle, addr, pdMS_TO_TICKS(50));
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Device found at address 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "Scan complete");
*/  
    //init HTU21D
    static htu21d_handle_t htu21d;
    ESP_ERROR_CHECK(htu21d_init(bus_handle, &htu21d));

    //sensor task
    xTaskCreatePinnedToCore(
        sensor_task,
        "sensor_task",
        4096,
        &htu21d,
        5,
        NULL,
        0
    );

    //display task
    display_task_params_t disp_params = {
        .bus = bus_handle
    };
    
    xTaskCreatePinnedToCore(
        display_task,
        "display_task",
        4096,
        &disp_params,
        4,
        NULL,
        1
    );

    ESP_LOGI(TAG, "System started");
    //Read
  //  htu21d_data_t data;
   // while(1){
   //     esp_err_t ret = htu21d_read_all(&htu21d, &data);
   //     if (ret == ESP_OK){
   //         ESP_LOGI(TAG, "Temperature: %.2f C",data.temperature);
   //         ESP_LOGI(TAG, "Humidity: %.2f %%",data.humidity);
   //     }
   //     else{
   //         ESP_LOGE(TAG, "Read Failed: %s", esp_err_to_name(ret));
   //     }
   //     vTaskDelay(pdMS_TO_TICKS(2000));
   // }
}
