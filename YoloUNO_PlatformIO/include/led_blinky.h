#ifndef __LED_BLINKY__
#define __LED_BLINKY__
#include <Arduino.h>
#include "global.h"
#define LED_GPIO 48
// Chân GPIO kết nối với LED trên bo mạch Yolo UNO (chip ESP32-S3)
#define LED_GPIO 48

// Task RTOS chung để chớp tắt LED (có thể dùng để báo hiệu hệ thống hoạt động)
void led_blinky(void *pvParameters);

// Task RTOS thực hiện Task 1: Thay đổi tần số/trạng thái nháy LED theo 3 mức nhiệt độ
void task1_led_blinky(void *pvParameters);
#endif