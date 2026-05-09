#pragma once

//----includes------------------------------------------------------------------------------------------------
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_task.h"
#include "freertos/task.h"

//----Task entry point------------------------------------------------------------------------------------------------
void mqtt_task(void *pvParameters);