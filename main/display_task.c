#include "display_task.h"
#include "ssd1306.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG  = "DISPLAY_TASK";

void display_task(void *pvParameters){
    display_task_params_t *params = (display_task_params_t *)pvParameters;

    //init
    ssd1306_config_t dev_cfg = I2C_SSD1306_128x64_CONFIG_DEFAULT;
    ssd1306_handle_t dev_handle;
    ssd1306_init (params->bus, &dev_cfg, &dev_handle);

    if (dev_handle == NULL){
        ESP_LOGE(TAG, "SSD1306 init failed");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Display task started");
    ssd1306_clear_display(dev_handle, false);

    //----startup screen-------------------------------
    ssd1306_display_text(dev_handle,0, "Smart Agri-Sys", false);
    ssd1306_display_text(dev_handle,1, "Starting", false);
    vTaskDelay(pdMS_TO_TICKS(2000));

    sensor_data_t data;
    char line[32];
    ssd1306_clear_display(dev_handle, false);
    while(1){
        if (xQueueReceive(q_display,&data, portMAX_DELAY) == pdTRUE){
            
            ssd1306_display_text(dev_handle,0, "Smart Agri-Sys", false);
            //display temp
            snprintf(line, sizeof(line), "Temp: %.1fC", data.temperature);
            ssd1306_display_text(dev_handle,2, line, false);
            
            //display Humidity
            snprintf(line, sizeof(line), "Hum: %.1f%%", data.humidity);
            ssd1306_display_text(dev_handle,4, line, false);

            //display timestamp
            snprintf(line, sizeof(line), "T: %" PRId64 "ms", data.timestamp_ms);
            ssd1306_display_text(dev_handle,6, line, false);
        }
    }

}