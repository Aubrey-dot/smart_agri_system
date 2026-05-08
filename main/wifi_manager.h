#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"

//----Event Bits--------------------------------
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

//----Shared event group------------------------
extern EventGroupHandle_t wifi_event_group;

//----Task-------------------------------------------------
esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_connect(char *wifi_ssid, char *wifi_password);
esp_err_t wifi_manager_disconnect(void);
esp_err_t wifi_manager_deinit(void);