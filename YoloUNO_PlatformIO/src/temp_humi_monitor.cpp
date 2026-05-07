#include "temp_humi_monitor.h"
#include "global.h"         
#include "task_webserver.h" 
#include <ArduinoJson.h>    
#include <esp_now.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Đối tượng điều khiển màn hình OLED qua giao thức I2C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Cấu trúc gói tin nhận qua ESP-NOW từ Gateway/Node khác
typedef struct struct_message {
    float temp;
    float humi;
} struct_message;

struct_message incomingReadings;

// Callback được kích hoạt mỗi khi nhận được gói tin ESP-NOW
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    
    EnvData_t myData;
    myData.temperature = incomingReadings.temp; 
    myData.humidity = incomingReadings.humi;    
    
    // Đẩy đè (Overwrite) dữ liệu mới nhất vào các Queue cho các Task khác xử lý
    if (qEnvDataAI != NULL) xQueueOverwrite(qEnvDataAI, &myData);
    if (qEnvDataMQTT != NULL) xQueueOverwrite(qEnvDataMQTT, &myData);
    if (qEnvDataLCD != NULL) xQueueOverwrite(qEnvDataLCD, &myData);
    if (qEnvDataLED != NULL) xQueueOverwrite(qEnvDataLED, &myData);
    
    // Nhả Semaphore để đánh thức Task LED (Task 1) cập nhật trạng thái
    if (xSemaphoreLED != NULL) {
        xSemaphoreGive(xSemaphoreLED);
    }

    Serial.printf("\n📥 [ESP-NOW] Nhận -> Nhiệt: %.1f°C | Ẩm: %.1f%%\n", myData.temperature, myData.humidity);

    // Bắn thẳng dữ liệu nhiệt/ẩm lên Web UI theo thời gian thực (Task 4)
    StaticJsonDocument<200> doc;
    doc["page"] = "home"; 
    doc["temp"] = myData.temperature;
    doc["humi"] = myData.humidity;
    String jsonString;
    serializeJson(doc, jsonString);
    Webserver_sendata(jsonString); 
}

// Task 3: Hiển thị thông số LCD/OLED và quản lý luồng dữ liệu trung tâm
void temp_humi_monitor(void *pvParameters){
    Serial.println("🚀 Khởi tạo ESP-NOW và OLED...");

    // Cấu hình chân I2C (SDA=11, SCL=12) cho bo mạch Yolo UNO (ESP32-S3)
    Wire.begin(11, 12);
    Wire.setClock(400000); // Set tốc độ I2C lên 400kHz cho mượt

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("❌ Lỗi: Không tìm thấy OLED");
    }

    display.setTextColor(SSD1306_WHITE);

    // Khởi tạo giao thức ESP-NOW và đăng ký callback nhận dữ liệu
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ Lỗi khởi tạo ESP-NOW");
    } else {
        esp_now_register_recv_cb(OnDataRecv);
    }
    
    EnvData_t lcdData;
    bool hasData = false;
    bool blink_state = false; // Biến phụ trợ để tạo hiệu ứng nhấp nháy cảnh báo

    while (1){
        // Cố gắng đọc dữ liệu mới nhất từ Queue OLED (chờ tối đa 10ms để không bị kẹt UI)
        if (xQueueReceive(qEnvDataLCD, &lcdData, pdMS_TO_TICKS(10)) == pdTRUE) {
            hasData = true;
        }
        blink_state = !blink_state; // Đảo trạng thái chớp tắt

        // Vẽ lại toàn bộ frame màn hình
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);

        // Header: Hiển thị trạng thái mạng (AP/STA) và IP
        if (WiFi.status() == WL_CONNECTED) {
            display.printf("MODE: STA | CH: %d\n", WiFi.channel());
            display.print("IP: "); 
            display.println(WiFi.localIP()); 
        } 
        else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            display.printf("MODE: AP | CH: %d\n", WiFi.channel());
            display.print("IP: "); 
            display.println(WiFi.softAPIP()); 
        }
        display.println("---------------------");

        // Body: Hiển thị thông số và trạng thái (Normal, Warning, Critical - Task 3)
        if (hasData) {
            // In Nhiệt độ và Độ ẩm dạng chữ to
            display.setTextSize(2);
            display.setCursor(0, 20);
            display.printf("T: %.1f C\n", lcdData.temperature);
            display.setCursor(0, 38);
            display.printf("H: %.1f %%\n", lcdData.humidity);

            display.setTextSize(1);
            int warningY = 56; 
            int warningH = 8;  

            // Ngưỡng cứng cảnh báo (Hard-coded threshold)
            float temp_threshold = 30.0;
            float humi_threshold = 70.0;

            // Logic hiển thị cảnh báo (Critical / Warning / Normal)
            if (lcdData.temperature > temp_threshold && lcdData.humidity > humi_threshold) {
                // Vẽ icon tam giác cảnh báo (Critical)
                int x_apex = 8,  y_apex = warningY - 1;
                int x_left = 0,  y_left = warningY + 7;
                int x_right = 16, y_right = warningY + 7;

                if (blink_state) {
                    display.fillTriangle(x_apex, y_apex, x_left, y_left, x_right, y_right, SSD1306_WHITE);
                    display.fillRect(7, warningY + 1, 3, 4, SSD1306_BLACK); 
                    display.drawPixel(8, warningY + 6, SSD1306_BLACK);    
                } else {
                    display.drawTriangle(x_apex, y_apex, x_left, y_left, x_right, y_right, SSD1306_WHITE);
                    display.fillRect(7, warningY + 1, 3, 4, SSD1306_WHITE); 
                    display.drawPixel(8, warningY + 6, SSD1306_WHITE);    
                }

                display.setCursor(22, warningY);
                display.print("!! NONG & AM CAO !!"); 
            } 
            else if (lcdData.temperature > temp_threshold) { // Warning Nhiệt độ
                display.drawRect(0, warningY - 1, 128, warningH + 2, SSD1306_WHITE);
                display.setCursor(10, warningY);
                display.print("! CANH BAO: NONG !");  
            } 
            else if (lcdData.humidity > humi_threshold) { // Warning Độ ẩm
                display.drawRect(0, warningY - 1, 128, warningH + 2, SSD1306_WHITE);
                display.setCursor(5, warningY);
                display.print("! CANH BAO: AM CAO !");
            } 
            else { // Normal
                display.setCursor(15, warningY);
                display.print("-> Binh thuong <-");   
            }

        } else {
            display.setCursor(0, 30);
            display.println("Waiting ESP-NOW...");
        }
        
        display.display(); // Push buffer ra màn hình thực tế
        
        vTaskDelay(pdMS_TO_TICKS(500)); // Refresh rate 2Hz
    }
}