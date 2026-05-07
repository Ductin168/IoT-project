#include "task_webserver.h"
#include <ArduinoJson.h>   
#include "task_check_info.h" 
#include "mqtt_client.h"

// Tạo instance Web Server chạy ở port 80 và WebSocket chạy ở path "/ws" (Task 4)
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

// Hàm gửi dữ liệu từ ESP32 lên tất cả các client đang kết nối WebSocket
void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data); 
        Serial.println("📤 Đã gửi dữ liệu qua WebSocket: " + data);
    }
    else
    {
        // Không có client nào đang mở giao diện web
    }
}

// Hàm phân tích và xử lý các lệnh JSON gửi từ Web Interface xuống ESP32
void handleWebSocketMessage(String message) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    if (error) return;

    String page = doc["page"].as<String>();
    
    // Xử lý luồng điều khiển thiết bị (Task 4)
    if (page == "device") {
        String name = doc["value"]["name"].as<String>();
        String status = doc["value"]["status"].as<String>();
        int gpio = doc["value"]["gpio"].as<int>();

        Serial.printf("\n📩 Nhận từ Web: %s\n", message.c_str());
        Serial.printf("⚙️ [%s] GPIO %d → %s\n", name.c_str(), gpio, status.c_str());

        // Thực thi lệnh vật lý điều khiển Relay/LED
        pinMode(gpio, OUTPUT);
        if (status == "ON") {
            digitalWrite(gpio, HIGH);
        } else {
            digitalWrite(gpio, LOW);
        }

        // Cập nhật ngược trạng thái này lên server MQTT (Đồng bộ đa nền tảng)
        sendDeviceStateToMQTT(gpio, status == "ON");
        
        // Trả kết quả (ACK) về lại cho Web để đổi màu nút bấm UI
        String response = "{\"page\":\"device_status\",\"gpio\":" + String(gpio) + ",\"status\":\"" + status + "\"}";
        Webserver_sendata(response);
    }
    // Xử lý luồng cấu hình WiFi và Server (Lưu thông tin)
    else if (page == "setting") {
        Serial.printf("\n⚙️ Nhận cấu hình từ Web: %s\n", message.c_str());
        String ssid = doc["value"]["ssid"].as<String>();
        String pass = doc["value"]["password"].as<String>();
        String server = doc["value"]["server"].as<String>();
        String port = doc["value"]["port"].as<String>();
    
        // Ghi xuống bộ nhớ Flash qua hàm của task_check_info
        Save_info_File(ssid, pass, "", server, port);

        Serial.println("🔄 Đang khởi động lại mạch sau 2 giây để áp dụng mạng mới...");
        
        // Tạo luồng delay ngắn để ESP32 kịp phản hồi WebSocket trước khi reset
        xTaskCreate([](void *pvParameters) {
            vTaskDelay(pdMS_TO_TICKS(2000)); 
            ESP.restart(); 
            vTaskDelete(NULL);
        }, "Restart_Task", 2048, NULL, 5, NULL);
    }
}

// ====================================================================

// Callback xử lý sự kiện kết nối/ngắt kết nối và nhận data của WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA) // Nhận được gói tin dạng text
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            handleWebSocketMessage(message); // Đẩy gói tin qua hàm xử lý nghiệp vụ
        }
    }
}

// Khởi tạo các Route cho Web Server và nạp file tĩnh từ LittleFS
void connnectWSV()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
              
    server.begin();
    ElegantOTA.begin(&server); // Bật tính năng cập nhật firmware qua OTA
    webserver_isrunning = true;
}

// Đóng máy chủ Web
void Webserver_stop()
{
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
}

// Tự động duy trì và khôi phục Web Server (thường được gọi trong loop)
void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop(); // Lắng nghe sự kiện nạp code không dây
}