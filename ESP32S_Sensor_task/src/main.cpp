#include <Arduino.h>
#include <WiFi.h> 
#include <esp_wifi.h> 
#include "global.h"
#include "temp_humi_monitor.h"

// Khai báo hàm 
void task_button_demo(void *pvParameters);

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); 
    esp_wifi_set_promiscuous(false);

    WiFi.disconnect(); 

    //Task Sensor
    xTaskCreate(temp_humi_monitor_task, "SensorTask", 4096, NULL, 1, NULL);
    
    //Task quản lý nút bấm
    xTaskCreate(task_button_demo, "ButtonTask", 2048, NULL, 1, NULL);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}