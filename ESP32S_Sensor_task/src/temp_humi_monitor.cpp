#include "temp_humi_monitor.h"
#include "global.h"
#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h> 

#define DHTPIN 23     
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE); 

uint8_t broadcastAddress[] = {0xCC, 0xBA, 0x97, 0x0D, 0xD3, 0x58}; 

typedef struct struct_message {
    float temp;
    float humi;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// =================================================================
// BIẾN TOÀN CỤC CHO TEST MODE
// =================================================================
bool is_test_mode = false;
float sim_temp = 25.0;
float sim_humi = 30.0;
bool edit_temp_mode = true; // true = Đang chỉnh Nhiệt độ, false = Đang chỉnh Độ ẩm

// =================================================================
// TASK 1: XỬ LÝ NÚT BẤM (GPIO 6) ĐỂ CHUYỂN TRẠNG THÁI
// =================================================================
void task_button_demo(void *pvParameters) {
    // Nên đổi sang chân GPIO 13 hoặc 14 để tránh các chân có tính năng Touch (như chân 4)
    const int BUTTON_PIN = 14; 
    pinMode(BUTTON_PIN, INPUT_PULLUP); 
    
    bool lastState = HIGH;
    unsigned long pressTime = 0;
    bool isPressing = false;
    bool handledLongPress = false;

    while(1) {
        bool reading = digitalRead(BUTTON_PIN);

        // --- CƠ CHẾ LỌC NHIỄU (DEBOUNCE) ---
        // Nếu thấy tín hiệu thay đổi, đợi 50ms rồi đọc lại xem có thực sự đổi không
        if (reading != lastState) {
            vTaskDelay(pdMS_TO_TICKS(50)); // Đợi rung động cơ khí ổn định
            reading = digitalRead(BUTTON_PIN); // Đọc lại lần nữa
        }

        // 1. Bắt đầu nhấn (Chuyển từ HIGH sang LOW)
        if (reading == LOW && lastState == HIGH) {
            pressTime = millis();
            isPressing = true;
            handledLongPress = false;
        }

        // 2. Đang giữ nút
        if (reading == LOW && isPressing) {
            unsigned long holdTime = millis() - pressTime;
            // Giữ đủ 10 giây để vào/thoát Test Mode
            if (holdTime >= 10000 && !handledLongPress) {
                is_test_mode = !is_test_mode;
                Serial.println(is_test_mode ? "\n🔥 ĐÃ VÀO TEST MODE" : "\n🌿 ĐÃ VỀ THỰC TẾ");
                handledLongPress = true;
            }
        }

        // 3. Nhả nút ra (Xử lý các lệnh dựa trên thời gian đã giữ)
        if (reading == HIGH && lastState == LOW) {
            unsigned long holdTime = millis() - pressTime;

            if (is_test_mode && !handledLongPress) {
                if (holdTime >= 1000) { // Giữ trên 1s: Đổi Mode Nhiệt/Ẩm
                    edit_temp_mode = !edit_temp_mode;
                    Serial.printf("\n🔄 Đổi sang chỉnh: %s\n", edit_temp_mode ? "NHIỆT ĐỘ" : "ĐỘ ẨM");
                } 
                else if (holdTime >= 100) { // Bấm nhanh (nhưng phải trên 100ms để tránh nhiễu)
                    if (edit_temp_mode) sim_temp += 1.0;
                    else sim_humi += 5.0;
                    Serial.printf("\n🔼 Tăng %s lên: %.1f\n", edit_temp_mode ? "Nhiệt" : "Ẩm", edit_temp_mode ? sim_temp : sim_humi);
                }
            }
            isPressing = false;
        }
        
        lastState = reading;
        vTaskDelay(pdMS_TO_TICKS(20)); // Nhịp quét 20ms
    }
}
// =================================================================
// TASK 2: ĐỌC DATA VÀ GỬI ĐI (ĐÃ TRỘN LOGIC CẦU DAO)
// =================================================================
void temp_humi_monitor_task(void *pvParameters) {
    Serial.println("\n--- BẮT ĐẦU KHỞI TẠO TASK ---");
    dht.begin();
    Serial.println("➤ Đã xong: Khởi tạo DHT11");

    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ Lỗi: Khởi tạo ESP-NOW");
    } else {
        Serial.println("➤ Đã xong: Khởi tạo ESP-NOW");
    }

    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    Serial.println("--- VÀO VÒNG LẶP ĐỌC DỮ LIỆU ---");

    while (1) {
        if (is_test_mode) {
            // TRƯỜNG HỢP A: ĐANG Ở TEST MODE -> Lấy Data Giả Lập
            myData.temp = sim_temp;
            myData.humi = sim_humi;
            Serial.printf("🧪 [GIẢ LẬP] Nhiệt độ: %.1f C, Độ ẩm: %.1f %%\n", myData.temp, myData.humi);
            esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        } 
        else {
            // TRƯỜNG HỢP B: BÌNH THƯỜNG -> Lấy Data Thực Tế
            float h = dht.readHumidity();
            float t = dht.readTemperature();

            if (isnan(h) || isnan(t)) {
                Serial.println("⚠️ Lỗi: Không thể lấy dữ liệu từ DHT11");
            } else {
                myData.temp = t;
                myData.humi = h;
                Serial.printf("🌍 [THỰC TẾ] Nhiệt độ: %.1f C, Độ ẩm: %.1f %%\n", myData.temp, myData.humi);
                esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
            }
        }

        // Vẫn duy trì nhịp độ gửi 5 giây/lần như cũ
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}