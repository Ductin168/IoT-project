#ifndef __TASK_TOOGLE_BOOT_H__
#define __TASK_TOOGLE_BOOT_H__

#include "global.h"
#include "task_check_info.h"

// Task RTOS lắng nghe và xử lý sự kiện nhấn nút BOOT trên bo mạch ESP32-S3
// Ứng dụng: Ví dụ nhấn giữ nút BOOT để xóa cấu hình WiFi/Token cũ và khởi động lại Access Point (Web Server)
void Task_Toogle_BOOT(void *pvParameters);

#endif