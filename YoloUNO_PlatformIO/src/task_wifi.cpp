#include "task_wifi.h"
#include "global.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_wifi.h> 

unsigned long last_wifi_check = 0;
const unsigned long WIFI_CHECK_INTERVAL = 300000; // Chu kỳ kiểm tra kết nối lại (5 phút)

// Hàm kích hoạt chế độ Access Point (để chạy Web Server nội bộ - Task 4)
void startAP()
{
    WiFi.disconnect(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.mode(WIFI_AP_STA); 
    
    // Ép AP phát ở Kênh 1 (để đảm bảo tính ổn định với ESP-NOW)
    String ap_pass = String(PASS_AP);
    if (ap_pass.length() > 0 && ap_pass.length() < 8) {
        WiFi.softAP(String(SSID_AP), "", 1); 
    } else {
        WiFi.softAP(String(SSID_AP), ap_pass.c_str(), 1);
    }
    
    // Ép driver ESP32 lock cố định vào channel 1
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    Serial.println("⚠️ Đã bật AP Mode (Kênh 1). IP: " + WiFi.softAPIP().toString());
}

// Hàm kết nối WiFi ở chế độ Station (để đẩy dữ liệu CoreIOT - Task 6)
void startSTA()
{
    String local_ssid = "";
    String local_pass = "";

    // Đọc thông tin SSID và Password từ file info.dat lưu trong LittleFS
    File file = LittleFS.open("/info.dat", "r");
    if (file) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        if (!error) {
            local_ssid = doc["WIFI_SSID"].as<String>();
            local_pass = doc["WIFI_PASS"].as<String>();
        }
        file.close();
    }

    local_ssid.trim();
    local_pass.trim();

    // Nếu không có thông tin mạng, rớt về chế độ AP
    if (local_ssid.isEmpty()) {
        startAP();
        return; 
    }

    Serial.println("📡 Đang kết nối WiFi: [" + local_ssid + "]");
    
    // Bắt buộc cấu hình kết nối Station ở kênh 1 để song hành với ESP-NOW
    WiFi.mode(WIFI_AP_STA); 
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    WiFi.begin(local_ssid.c_str(), local_pass.c_str(), 1);

    int retry_count = 0;
    while (WiFi.status() != WL_CONNECTED && retry_count < 40) { // Timeout 20 giây
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        retry_count++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        WiFi.setSleep(false); // Tắt tính năng ngủ của WiFi để tránh rớt gói tin MQTT
        esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); 
        Serial.printf("✅ WiFi OK! IP: %s | Kênh: %d\n", WiFi.localIP().toString().c_str(), WiFi.channel());
        
        // Báo hiệu Semaphore cho các task khác (MQTT) biết đã có internet
        if (xBinarySemaphoreInternet != NULL) {
            xSemaphoreGive(xBinarySemaphoreInternet);
        }
    } else {
        Serial.println("❌ Không tìm thấy WiFi ở Kênh 1 -> Về AP Mode!");
        startAP(); 
    }
}

// Hàm giám sát tự động khôi phục kết nối WiFi
bool Wifi_reconnect()
{
    if (WiFi.status() == WL_CONNECTED) {
        if (WiFi.channel() != 1) {
            esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); // Cố định channel ESP-NOW
        }
        return true;
    }

    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        if (millis() - last_wifi_check >= WIFI_CHECK_INTERVAL) {
            last_wifi_check = millis(); 
            startSTA(); // Thử kết nối lại STA sau mỗi 5 phút
        }
        return false; 
    }
    startSTA();
    return false;
}

// Task chạy ngầm RTOS quản lý trạng thái mạng
void task_wifi(void *pvParameters) {
    while (1) {
        Wifi_reconnect();
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}