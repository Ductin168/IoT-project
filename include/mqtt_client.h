#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
void mqtt_setup();
void mqtt_loop();
void sendTelemetry(PubSubClient& client, const char* dev_name, int temp, int humi, int light, bool is_dev1);

// thêm 2 hàm này
void pushRandomTelemetry(uint32_t delay_ms);
void task_push_mqtt(void *pvParameters);