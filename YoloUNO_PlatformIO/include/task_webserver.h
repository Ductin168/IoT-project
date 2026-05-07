
#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>

// Đối tượng máy chủ Web bất đồng bộ (chạy ở chế độ Access Point theo Task 4)
extern AsyncWebServer server;

// Đối tượng WebSocket dùng để truyền nhận dữ liệu thời gian thực giữa Web và ESP32-S3
extern AsyncWebSocket ws;

// Hàm dừng hoạt động của Web Server một cách an toàn
void Webserver_stop();

// Hàm khởi động lại hoặc kết nối lại các dịch vụ của Web Server
void Webserver_reconnect();

// Hàm gửi dữ liệu (thường ở định dạng JSON) qua WebSocket xuống giao diện người dùng
// @param data: Chuỗi dữ liệu chứa thông tin cập nhật (nhiệt độ, trạng thái thiết bị...)
void Webserver_sendata(String data);

#endif