#include "global.h"
#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include <LittleFS.h>
#include "mqtt_client.h"
#include "tinyml.h"

void setup()
{
    Serial.begin(115200);
    delay(1000); 
    
    Serial.println("\n\n=======================================");
    Serial.println("🚀 BẮT ĐẦU KHỞI ĐỘNG HỆ THỐNG (TASK 3)...");
    Serial.println("=======================================");

    Serial.println("[1] Bật nguồn Ăng-ten (Radio)...");
    WiFi.mode(WIFI_STA);
    // LittleFS được dùng để lưu trữ file cấu hình tĩnh (HTML, CSS) và thông tin cấu hình mạng/Token
    Serial.println("[2] Đang Mount ổ cứng LittleFS...");
    if (!LittleFS.begin(true)) {
        Serial.println("❌ Lỗi Mount LittleFS!");
    } else {
        Serial.println("✅ Mount LittleFS thành công.");
    }
    // Khởi tạo các Queue (kích thước 1 phần tử) để truyền dữ liệu EnvData_t giữa các Task an toàn
    Serial.println("[4] Cấp phát bộ nhớ Queue/Semaphore...");
    qEnvDataAI = xQueueCreate(1, sizeof(EnvData_t));
    qEnvDataMQTT = xQueueCreate(1, sizeof(EnvData_t));
    qEnvDataLCD = xQueueCreate(1, sizeof(EnvData_t));
    qEnvDataLED = xQueueCreate(1, sizeof(EnvData_t));
    qEnvDataNeo = xQueueCreate(1, sizeof(EnvData_t));
    
    qLedCommand = xQueueCreate(1, sizeof(bool));
    // Khởi tạo Binary Semaphore để bảo vệ tài nguyên LED và NeoPixel (Yêu cầu của Task 1 & 2)
    xSemaphoreLED = xSemaphoreCreateBinary();
    xSemaphoreNeo = xSemaphoreCreateBinary();

    Serial.println("[5] Khởi tạo cấu hình MQTT...");
    mqtt_setup(); 

    Serial.println("[6] Bắn các Task đa luồng (FreeRTOS)...");
    // Phân bổ các luồng độc lập, ưu tiên các Task có độ quan trọng cao (như WiFi, MQTT)
    xTaskCreate(task_wifi, "Task WiFi", 4096, NULL, 3, NULL);
    xTaskCreate(led_blinky, "Task LED 48", 2048, NULL, 2, NULL);
    xTaskCreate(task1_led_blinky, "Task 1 LED", 2048, NULL, 2, NULL);
    xTaskCreate(neo_blinky, "Task NeoPixel", 2048, NULL, 2, NULL);
    xTaskCreate(temp_humi_monitor, "Task Temp Humi", 4096, NULL, 2, NULL);
    xTaskCreate(Task_Toogle_BOOT, "Task Toggle BOOT", 2048, NULL, 2, NULL);  
    xTaskCreate(task_push_mqtt, "Task Push MQTT", 8192, NULL, 2, NULL);  
    xTaskCreate(tiny_ml_task, "Task TinyML", 4096, NULL, 2, NULL);
    // Task ẩn quản lý kết nối và tự động khôi phục Web Server (Task 4)
    xTaskCreate([](void *pvParameters) {
        while(1) {
            Webserver_reconnect(); 
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }, "Web_Server_Task", 4096, NULL, 1, NULL);
}

void loop()
{
    // Để trống vòng lặp loop() do hệ thống đã hoàn toàn được quản lý bởi FreeRTOS
    vTaskDelay(pdMS_TO_TICKS(1000));
}