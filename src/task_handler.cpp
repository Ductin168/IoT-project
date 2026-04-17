#include "task_handler.h"
#include <ArduinoJson.h>    // Giải quyết 90% đống lỗi JSON
#include "global.h"         // Giải quyết lỗi Save_info_File và biến ledBlinkEnabled
#include "task_webserver.h" // Giải quyết lỗi biến ws (WebSocket)
#include "mqtt_client.h"    // Để gọi được MQTT gửi lên CoreIOT
#include <PubSubClient.h>

extern PubSubClient pubSubClient1;

void handleWebSocketMessage(String message)
{
    Serial.println("📩 Nhận từ Web: " + message);
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.println("❌ Lỗi parse JSON!");
        return;
    }
    
    String page = doc["page"].as<String>();
    
    // ========================================================
    // 1. XỬ LÝ LỆNH BẬT/TẮT THIẾT BỊ (ĐỘNG)
    // ========================================================
    if (page == "device")
    {
        JsonObject value = doc["value"];
        if (!value.containsKey("gpio") || !value.containsKey("status") || !value.containsKey("name"))
        {
            Serial.println("⚠️ JSON thiếu thông tin gpio, status hoặc name");
            return;
        }

        int gpio = value["gpio"];
        String status = value["status"].as<String>();
        String dev_name = value["name"].as<String>(); // 👉 Giữ nguyên tên sếp gõ (Hoa/Thường)

        Serial.printf("⚙️ Điều khiển [%s] GPIO %d → %s\n", dev_name.c_str(), gpio, status.c_str());
        pinMode(gpio, OUTPUT);
        
        bool is_on = status.equalsIgnoreCase("ON");

        if (is_on)
        {
            digitalWrite(gpio, HIGH);
            Serial.printf("🔆 [%s] GPIO %d ON\n", dev_name.c_str(), gpio);
            
            // Báo cáo ngược lại cho Web để Web đổi nút sang màu Xanh (BẬT)
            Webserver_sendata("{\"page\":\"device_status\",\"gpio\":" + String(gpio) + ",\"status\":\"ON\"}");
        }
        else 
        {
            digitalWrite(gpio, LOW);
            Serial.printf("💤 [%s] GPIO %d OFF\n", dev_name.c_str(), gpio);
            
            // Báo cáo ngược lại cho Web để Web đổi nút sang màu Xám (TẮT)
            Webserver_sendata("{\"page\":\"device_status\",\"gpio\":" + String(gpio) + ",\"status\":\"OFF\"}");
        }

        // --- ĐỒNG BỘ ĐỘNG LÊN CORE IOT ---
        if (pubSubClient1.connected()) {
            // Xóa khoảng trắng để chuẩn form JSON, nhưng KHÔNG viết thường nữa
            dev_name.replace(" ", "");

            // 👉 CHÌA KHÓA Ở ĐÂY: Chỉ kích hoạt chế độ nháy (Task 1) nếu GPIO đúng bằng 48
            // if (gpio == 48) {
            //     ledBlinkEnabled = is_on; 
            // }

            // Tự tay nối chuỗi JSON bằng đúng cái tên sếp gõ
            String payload = "{\"" + dev_name + "\":" + (is_on ? "true" : "false") + "}";

            // Chỉ gửi vào Attributes (Chuẩn IoT, không vứt rác sang Telemetry)
            pubSubClient1.publish("v1/devices/me/attributes", payload.c_str());

            Serial.println("☁️ Đã đồng bộ thiết bị lên CoreIOT: " + payload);
        }
    }
    
    // ========================================================
    // 2. YÊU CẦU LẤY TRẠNG THÁI (KHI WEB VỪA MỞ LÊN/F5)
    // ========================================================
    else if (page == "get_status") 
    {
        Serial.println("🔄 Web Local vừa load, đang đồng bộ trạng thái UI...");
        // Đọc trạng thái thực tế của chân 48
        String stat = digitalRead(48) == HIGH ? "ON" : "OFF";
        Webserver_sendata("{\"page\":\"device_status\",\"gpio\":48,\"status\":\"" + stat + "\"}");
    }
    
    // ========================================================
    // 3. XỬ LÝ LƯU CẤU HÌNH WIFI/TOKEN
    // ========================================================
    else if (page == "setting")
    {
        JsonObject value = doc["value"];
        String WIFI_SSID = value["ssid"].as<String>();
        String WIFI_PASS = value["password"].as<String>();
        String CORE_IOT_TOKEN = value["token"].as<String>();
        String CORE_IOT_SERVER = value["server"].as<String>();
        String CORE_IOT_PORT = value["port"].as<String>();

        Serial.println("📥 Nhận cấu hình từ WebSocket");
        // Gọi hàm lưu cấu hình
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Phản hồi lại client
        String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
        ws.textAll(msg);
    }
}