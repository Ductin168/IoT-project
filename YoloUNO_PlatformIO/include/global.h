#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h" 
// Cấu trúc dữ liệu chứa thông tin môi trường thu thập từ cảm biến
typedef struct {
    float temperature;
    float humidity;
} EnvData_t;
// === KHAI BÁO CÁC HÀNG ĐỢI (QUEUE) TRONG RTOS ===
// Sử dụng Queue để truyền dữ liệu an toàn giữa các Task với nhau
extern QueueHandle_t qEnvData;    
extern QueueHandle_t qEnvDataAI;  
extern QueueHandle_t qEnvDataMQTT;
extern QueueHandle_t qEnvDataLCD;
extern QueueHandle_t qEnvDataLED;
extern QueueHandle_t qEnvDataNeo;
extern QueueHandle_t qLedCommand; 
// === KHAI BÁO CÁC SEMAPHORE VÀ MUTEX ===
// Binary Semaphore để báo hiệu trạng thái kết nối Internet (có mạng mới cho publish data)
extern SemaphoreHandle_t xBinarySemaphoreInternet;
// Semaphore để đồng bộ hóa việc điều khiển LED, tránh đụng độ tài nguyên
extern SemaphoreHandle_t xSemaphoreLED;// Quản lý quyền cập nhật trạng thái LED đơn (Task 1)
extern SemaphoreHandle_t xSemaphoreNeo;// Quản lý quyền cập nhật màu sắc LED NeoPixel (Task 2)
// Mutex để bảo vệ tài nguyên kết nối WiFi/Mạng, đảm bảo chỉ 1 task được dùng mạng tại 1 thời điểm
extern SemaphoreHandle_t wifiMutex;

#endif