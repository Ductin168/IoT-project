#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "global.h"

// Task RTOS thực hiện Task 3:
// 1. Đọc dữ liệu định kỳ từ cảm biến DHT20.
// 2. Phân phối dữ liệu vào các Queue (cho AI, MQTT, LED, LCD).
// 3. Hiển thị thông số và các trạng thái (Normal, Warning, Critical) lên màn hình LCD thông qua giao tiếp I2C.
void temp_humi_monitor(void *pvParameters);

#endif