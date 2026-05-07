#ifndef __TASK_WIFI_H__
#define __TASK_WIFI_H__

#include <WiFi.h>
#include <task_check_info.h>
#include <task_webserver.h>

// Hàm kiểm tra và thực hiện kết nối lại WiFi nếu bị mất kết nối (Station Mode)
extern bool Wifi_reconnect();

// Hàm khởi tạo ESP32-S3 ở chế độ Access Point (phát WiFi)
// Phục vụ cho Task 4: Chạy Web Server trực tiếp trên ESP32
extern void startAP();

// Task RTOS chuyên trách việc duy trì kết nối WiFi ổn định trong suốt vòng đời hoạt động
extern void task_wifi(void *pvParameters);
#endif