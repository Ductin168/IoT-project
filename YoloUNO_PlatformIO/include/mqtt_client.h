#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
// Hàm khởi tạo các cấu hình ban đầu cho kết nối MQTT (Broker, Port, Callback)
void mqtt_setup();

// Hàm duy trì kết nối MQTT và xử lý các gói tin đến (cần gọi thường xuyên)
void mqtt_loop();

// Hàm đóng gói và gửi dữ liệu cảm biến (Telemetry) lên MQTT Broker
// @param dev_name: Tên thiết bị gửi
// @param is_dev1: Cờ xác định thiết bị (hỗ trợ điều khiển nhiều thiết bị)
void sendTelemetry(PubSubClient& client, const char* dev_name, int temp, int humi, int light, bool is_dev1);

// Hàm gửi dữ liệu mô phỏng lên server (dùng cho mục đích test khi không có cảm biến thực)
void pushRandomTelemetry(uint32_t delay_ms);

// Task RTOS chuyên trách việc đẩy dữ liệu lên MQTT theo định kỳ
void task_push_mqtt(void *pvParameters);

// Hàm báo cáo trạng thái hiện tại của thiết bị (bật/tắt chân GPIO) lên MQTT
void sendDeviceStateToMQTT(int gpio, bool state);