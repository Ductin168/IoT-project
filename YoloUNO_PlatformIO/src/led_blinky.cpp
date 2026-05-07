#include "led_blinky.h"
#include "global.h"

// Chân GPIO điều khiển LED tích hợp trên bo Yolo UNO (Sử dụng chip ESP32-S3)
#define LED_GPIO 48

// Task phụ trợ: Dùng để nháy LED cơ bản báo hiệu thiết bị đang sống (Heartbeat)
// Hoặc nhận lệnh bật/tắt từ Queue điều khiển của WebServer
void led_blinky(void *pvParameters){
    pinMode(LED_GPIO, OUTPUT);
    bool currentEnabled = true;

    while(1) {
        // Nếu có lệnh mới từ qLedCommand, cập nhật trạng thái cờ currentEnabled
        if (qLedCommand != NULL) {
            xQueueReceive(qLedCommand, &currentEnabled, 0);
        }

        // Nếu được phép chạy, tiến hành nháy LED chu kỳ 1s (500ms ON / 500ms OFF)
        if (currentEnabled) {
            digitalWrite(LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            // Tắt hẳn LED nếu nhận lệnh False
            digitalWrite(LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Chân điều khiển LED gắn ngoài cho Task 1 (tùy chỉnh lại cấu hình phần cứng nếu cần)
#define TASK1_LED_PIN 1

// Task 1: Thay đổi tần số và trạng thái nháy LED theo 3 điều kiện nhiệt độ 
void task1_led_blinky(void *pvParameters) {
    pinMode(TASK1_LED_PIN, OUTPUT);
    EnvData_t data;

    int delay_time = 100; 
    bool led_state = LOW; 

    while (1) {
        // Xin quyền truy cập Semaphore trước khi truy vấn hàng đợi
        if (xSemaphoreTake(xSemaphoreLED, pdMS_TO_TICKS(10)) == pdTRUE) {
            
            // Rút dữ liệu môi trường từ Queue
            if (qEnvDataLED != NULL && xQueueReceive(qEnvDataLED, &data, 0) == pdTRUE) {
                // Định nghĩa 3 mức độ hành vi cảnh báo (Task 1)
                if (data.temperature < 28.0) delay_time = -1;         // Dưới 28 độ: Tắt hẳn LED
                else if (data.temperature <= 32.0) delay_time = 300;  // 28-32 độ: Nháy LED với delay 300ms
                else delay_time = 0;                                  // Trên 32 độ: Bật LED sáng liên tục
            }
            
            // (Lưu ý: Không có lệnh xSemaphoreGive ở đây có thể gây Deadlock nếu có Task khác muốn lấy)
            // (Giữ nguyên luồng code cũ theo yêu cầu của bạn)
        }

        // Logic thực thi vật lý dựa trên delay_time đã tính toán
        if (delay_time == -1) {
            digitalWrite(TASK1_LED_PIN, LOW); // Tắt LED
            vTaskDelay(pdMS_TO_TICKS(100)); 
        } 
        else if (delay_time == 0) {
            digitalWrite(TASK1_LED_PIN, HIGH); // Sáng liên tục
            vTaskDelay(pdMS_TO_TICKS(100)); 
        } 
        else {
            led_state = !led_state; 
            digitalWrite(TASK1_LED_PIN, led_state); // Nhấp nháy
            vTaskDelay(pdMS_TO_TICKS(delay_time)); 
        }
    }
}