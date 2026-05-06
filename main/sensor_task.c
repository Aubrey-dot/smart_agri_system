#include "sensor_task.h"
#include "esp_log.h"
#include "esp_timer.h"


static const char *TAG = "SENSOR_TASK";

//----Queues------------------------------------
QueueHandle_t q_logger = NULL;
QueueHandle_t q_mqtt = NULL;
QueueHandle_t q_display = NULL;

//----Task---------------------------------------
void sensor_task(void *pvParameters){
    htu21d_handle_t *htu21d = (htu21d_handle_t *)pvParameters;

    q_logger = xQueueCreate(10, sizeof(sensor_data_t));
    q_mqtt   = xQueueCreate(10, sizeof(sensor_data_t));
    q_display   = xQueueCreate(10, sizeof(sensor_data_t));

    if (q_logger == NULL || q_mqtt == NULL || q_display == NULL){
        ESP_LOGE(TAG, "Failed to create queues");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Sensor task started");

    sensor_data_t data;

    while(1){
        data.timestamp_ms = esp_timer_get_time()/1000;

        //read sensor
        esp_err_t ret = htu21d_read_all(htu21d,(htu21d_data_t *)&data.temperature);
        if (ret == ESP_OK){
            xQueueSend(q_logger, &data, 0);
            xQueueSend(q_mqtt, &data, 0);
            xQueueSend(q_display, &data, 0);

            ESP_LOGI(TAG, "%.2f %.2f t=%" PRId64 "ms", data.temperature, data.humidity, data.timestamp_ms);
        }
        else{
            ESP_LOGE(TAG, "Read Failed: ", esp_err_to_name(ret));
        }
    }

    //wait 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));

}