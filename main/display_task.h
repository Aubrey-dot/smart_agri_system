#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "sensor_task.h"

//----Display params-------------------------
typedef struct{
    i2c_master_bus_handle_t bus;
} display_task_params_t;

//----Task------------------------------------
void display_task(void *pvParameters);