#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
//extern SemaphoreHandle_t xCountingSemaphoreInternet;
//extern QueueHandle_t xQueue;


struct SensorData {
    float temp;
    float humid;
};

typedef struct {
    QueueHandle_t xSensorQueue; // queue chứa data: temp và humi
    QueueHandle_t xLedSem;     // Cho Task 1
    QueueHandle_t xNeoSem;     // Cho Task 2
    //SemaphoreHandle_t xISRSem;     
    // Watchdog Semaphores
    //SemaphoreHandle_t xWD_Sensor;
    //SemaphoreHandle_t xWD_UI;
    //SemaphoreHandle_t xMutex;

} SystemResources;
//extern SystemResources res;
//void initGlobalResources();

#endif