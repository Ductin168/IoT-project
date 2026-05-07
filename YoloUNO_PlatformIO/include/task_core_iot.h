#ifndef __TASK_CORE_IOT_H__
#define __TASK_CORE_IOT_H__

#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <HTTPClient.h>
#include "task_check_info.h"
// Hàm thực hiện publish dữ liệu lên server CoreIOT (Task 6)
// @param mode: Chế độ gửi (ví dụ: telemetry)
// @param feed: Tên feed/topic dữ liệu tương ứng trên hệ thống CoreIOT
// @param data: Giá trị dữ liệu cần gửi (đã chuyển sang chuỗi)
void CORE_IOT_sendata(String mode, String feed, String data);
// Hàm kiểm tra và tự động kết nối lại với máy chủ CoreIOT nếu bị mất kết nối
void CORE_IOT_reconnect();

#endif