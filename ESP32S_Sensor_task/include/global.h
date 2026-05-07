#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Các biến lưu cấu hình
extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

// Semaphore để báo hiệu có Internet
extern SemaphoreHandle_t xBinarySemaphoreInternet;

// Nếu mạch A cần dùng Queue để truyền nhận dữ liệu nội bộ thì khai báo ở đây
// Mạch A hiện tại chủ yếu là đọc sensor và gửi đi ngay nên có thể chưa cần Queue phức tạp

#endif