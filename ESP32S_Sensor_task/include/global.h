#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Semaphore để báo hiệu trạng thái Internet (nếu sau này cần mở rộng)
extern SemaphoreHandle_t xBinarySemaphoreInternet;

// Ghi chú: Mạch A hiện tại chủ yếu là đọc sensor và gửi đi ngay qua ESP-NOW 
// nên đã loại bỏ các biến cấu hình WiFi/Cloud không sử dụng.

#endif