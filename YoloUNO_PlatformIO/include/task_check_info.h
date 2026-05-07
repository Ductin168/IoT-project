#ifndef __TASK_CHECK_INFO_H__
#define __TASK_CHECK_INFO_H__

#include <ArduinoJson.h>
#include "LittleFS.h"
#include "global.h"
#include "task_wifi.h"


// Hàm kiểm tra xem file cấu hình hệ thống (chứa token, wifi) có tồn tại trong bộ nhớ Flash (LittleFS) hay không
bool check_info_File(bool check);

// Hàm đọc dữ liệu cấu hình từ file trong bộ nhớ Flash và nạp vào các biến hệ thống
void Load_info_File();

// Hàm xóa file cấu hình hiện tại (thường dùng khi reset thiết bị về trạng thái ban đầu)
void Delete_info_File();

// Hàm ghi thông tin cấu hình mạng và CoreIOT xuống file trong bộ nhớ Flash dưới định dạng JSON
void Save_info_File(String WIFI_SSID, String WIFI_PASS, String CORE_IOT_TOKEN, String CORE_IOT_SERVER, String CORE_IOT_PORT);
#endif