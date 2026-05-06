#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"
#include "htu21d.h"

//----Sensor Data-------------------------------------
typedef struct {
    int64_t timestamp_ms;
    float temperature;
    float humidity;
} sensor_data_t;

//-----Queue handles---------------------------------
extern QueueHandle_t q_logger;
extern QueueHandle_t q_mqtt;
extern QueueHandle_t q_display;

//-----Task Entry point------------------------------
void sensor_task(void *pvParameters);

