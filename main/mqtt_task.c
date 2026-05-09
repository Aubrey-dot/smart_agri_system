#include "mqtt_task.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_event.h"
#include "inttypes.h"
#include "app_config.h"
#include "sensor_task.h"

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;

//----event handler---------------------------------------------------------------------------------------------
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
esp_mqtt_event_handle_t event = event_data;
esp_mqtt_client_handle_t client = event->client;
switch((esp_mqtt_event_id_t)event_id){
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT client connected");
        esp_mqtt_client_publish(client, MQTT_TOPIC_STATUS,"Online",0,1,0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT client diconnected");
        break;
    default:
        ESP_LOGI(TAG, "Other event id: %d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void){

    esp_mqtt_client_config_t mqtt_cfg ={
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client,ESP_EVENT_ANY_ID,mqtt_event_handler,NULL);
    esp_mqtt_client_start(client);
    }

//----MQTT Task--------------------------------------------------------------------------
void mqtt_task(void *pvParameters){ 
    mqtt_app_start();
    sensor_data_t data;
    char json_payload[128];

    while(1){
        if (xQueueReceive(q_mqtt,&data, portMAX_DELAY) == pdTRUE){
            snprintf(json_payload,sizeof(json_payload),
            "{\"temperature\":%.2f,\"humidity\":%.2f,\"timestamp\":%lld}",
            data.temperature, data.humidity, data.timestamp_ms);

            int msg_id = esp_mqtt_client_publish(client,MQTT_TOPIC_TEMP, json_payload,0,1,0);
            ESP_LOGI(TAG, "Published to %s, msg_id=%d, payload=%s", MQTT_TOPIC_TEMP, msg_id, json_payload);
        }

    }
}