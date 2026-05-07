#include "neo_blinky.h"
#include "global.h" 

// Khởi tạo đối tượng điều khiển dải LED NeoPixel (WS2812B)
// LED_COUNT, NEO_PIN được cấu hình tĩnh bên file neo_blinky.h
Adafruit_NeoPixel pixels(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Task RTOS thực hiện Task 2: Điều khiển NeoPixel
void neo_blinky(void *pvParameters) {
    pixels.begin(); // Khởi tạo chân GPIO giao tiếp với NeoPixel
    pixels.clear(); // Tắt toàn bộ LED lúc hệ thống vừa khởi động
    pixels.show();  // Cập nhật trạng thái tắt ra phần cứng thực tế

    EnvData_t ledData;
    float humi = 50.0; // Giá trị độ ẩm khởi tạo mặc định
    while(1) {
        // Xin quyền truy cập Semaphore trước khi điều khiển để tránh đụng độ tài nguyên
        if (xSemaphoreTake(xSemaphoreLED, pdMS_TO_TICKS(100)) == pdTRUE) {
        
            // Đọc dữ liệu độ ẩm từ Queue (dùng Peek để copy data mà không xóa nó khỏi hàng đợi)
            if (qEnvDataLED != NULL && xQueuePeek(qEnvDataLED, &ledData, 0) == pdTRUE) {
                humi = ledData.humidity;
                
                // Logic Mapping 3 khoảng độ ẩm thành 3 màu sắc khác nhau theo yêu cầu
                if (humi < 40.0) {
                    pixels.setPixelColor(0, pixels.Color(20, 0, 0)); // Màu đỏ (Môi trường khô)
                } 
                else if (humi >= 40.0 && humi <= 70.0) {
                    pixels.setPixelColor(0, pixels.Color(0, 20, 0)); // Màu xanh lá (Độ ẩm lý tưởng)
                } 
                else {
                    pixels.setPixelColor(0, pixels.Color(0, 0, 20)); // Màu xanh dương (Môi trường ẩm cao)
                }
                
                // Gửi tín hiệu điện ra dải LED để thay đổi màu ngay lập tức
                pixels.show();
                
            // Trả lại Semaphore cho hệ thống để các Task khác có thể sử dụng LED
            xSemaphoreGive(xSemaphoreLED);
        }
        
        // Ngủ 50ms để nhường thời gian CPU (Yield) cho các Task có độ ưu tiên thấp hơn
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}
}