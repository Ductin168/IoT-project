#include "mqtt_client.h"
#include "global.h"
#include "task_webserver.h" 
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <Preferences.h> 
#include <LittleFS.h> 

// Các biến toàn cục quản lý giao thức mạng MQTT
WiFiClient mqttEspClient1;
PubSubClient pubSubClient1(mqttEspClient1);

// Dùng bộ nhớ Preferences để lưu trạng thái Relay an toàn khi cúp điện
Preferences relay_prefs; 

// Hàm đẩy trạng thái thực tế của thiết bị (GPIO) lên MQTT Broker
void sendDeviceStateToMQTT(int gpio, bool state) {
    if (!pubSubClient1.connected()) return;
    
    StaticJsonDocument<128> doc;
    String key = "gpio_" + String(gpio);
    doc[key] = state;
    
    char buffer[128];
    serializeJson(doc, buffer);
    
    // Gửi bản tin Attribute để cập nhật trạng thái hiển thị trên Dashboard ThingsBoard/CoreIOT
    pubSubClient1.publish("v1/devices/me/attributes", buffer); 
    Serial.printf("📤 [MQTT] Đã Push trạng thái %s = %s lên Broker\n", key.c_str(), state ? "ON" : "OFF");
}

// Hàm xử lý lệnh điều khiển từ xa (RPC Call) từ CoreIOT/ThingsBoard đẩy xuống
void processRPC(PubSubClient& client, int dev_id, char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (int i = 0; i < length; i++) msg += (char)payload[i];

    Serial.println("\n--- [NHAN LENH TU GATEWAY] ---");
    Serial.println("Topic: " + String(topic));
    Serial.println("Payload: " + msg);

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) return;

    String methodName = doc["method"].as<String>(); 
    bool value = doc["params"]; 
    
    // Logic ánh xạ lệnh RPC sang thao tác thay đổi mức logic GPIO tương ứng
    if (methodName.startsWith("gpio_")) {
        int gpio = methodName.substring(5).toInt(); 
        pinMode(gpio, OUTPUT);

        bool currentState = digitalRead(gpio); 

        // Chỉ thực thi nếu trạng thái yêu cầu khác với trạng thái hiện tại
        if (currentState != value) {
            digitalWrite(gpio, value ? HIGH : LOW);
            
            // Lưu trạng thái vào flash (NVS)
            relay_prefs.begin("relay_data", false);
            relay_prefs.putBool(methodName.c_str(), value);
            relay_prefs.end();
            
            // Đồng bộ trạng thái về ngược lại Web Server (WebSocket) để UI thay đổi
            String stat = value ? "ON" : "OFF";
            Webserver_sendata("{\"page\":\"device_status\",\"gpio\":" + String(gpio) + ",\"status\":\"" + stat + "\"}");
            
            // Trả xác nhận ngược lên MQTT
            sendDeviceStateToMQTT(gpio, value); 

            Serial.printf(">>> ⚙️ THUC THI CHINH THUC: GPIO %d -> %s\n", gpio, stat.c_str());
        }
    }
}

// Callback wrapper được PubSubClient gọi khi có gói tin tới
void mqttCallback1(char* topic, byte* payload, unsigned int length) {
    processRPC(pubSubClient1, 1, topic, payload, length);
}

// Đọc thông số máy chủ MQTT từ cấu hình và thiết lập ban đầu (Task 6)
void mqtt_setup() {
    static String local_server = ""; 
    int local_port = 1883;

    File file = LittleFS.open("/info.dat", "r");
    if (file) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        if (!error) {
            local_server = doc["CORE_IOT_SERVER"].as<String>();
            String port_str = doc["CORE_IOT_PORT"].as<String>();
            if(port_str.length() > 0) local_port = port_str.toInt();
        }
        file.close();
    }

    if (local_server != "" && local_server != "null") {
        Serial.println("📡 MQTT Broker: " + local_server + " Port: " + String(local_port));
        pubSubClient1.setServer(local_server.c_str(), local_port);
    } else {
        Serial.println("⚠️ CẢNH BÁO: Chưa có địa chỉ MQTT Broker trong info.dat!");
    }
    
    pubSubClient1.setCallback(mqttCallback1);
    pubSubClient1.setBufferSize(512); // Tăng buffer chống tràn khi nhận chuỗi JSON dài
}

// Vòng lặp duy trì kết nối và khôi phục MQTT (Keep-alive)
void mqtt_loop() {
    if (WiFi.status() != WL_CONNECTED) return; // Nếu mất wifi thì không thử kết nối lại MQTT

    if (!pubSubClient1.connected()) {
        Serial.print("Reconnecting MQTT...");
        
        String local_token = "Mach_B_Gateway_Node"; // Device Token dự phòng
        File file = LittleFS.open("/info.dat", "r");
        if (file) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, file);
            if (!error && doc.containsKey("CORE_IOT_TOKEN")) {
                local_token = doc["CORE_IOT_TOKEN"].as<String>();
            }
            file.close();
        }

        // Đăng nhập vào broker (dùng Token cho cả Username/Password như chuẩn của CoreIOT)
        if (pubSubClient1.connect(local_token.c_str(), local_token.c_str(), NULL)) {
            Serial.println("Connected!");
            // Đăng ký nhận (Subscribe) gói tin điều khiển RPC
            pubSubClient1.subscribe("v1/devices/me/rpc/request/+");
        } else {
            Serial.println("Failed!");
            vTaskDelay(pdMS_TO_TICKS(2000)); 
        }
    }
    
    // Cần gọi liên tục để thư viện xử lý gói tin đến và gửi ping
    if (pubSubClient1.connected()) {
        pubSubClient1.loop();
    }
}

// Hàm format và đóng gói dữ liệu cảm biến (Telemetry) đẩy lên Cloud (Task 6)
void sendTelemetry(PubSubClient& client, float temp, float humi) {
    if (!client.connected()) return;
    StaticJsonDocument<128> doc;
    doc["temperature"] = temp;
    doc["humidity"] = humi;
    char buffer[128];
    serializeJson(doc, buffer);
    client.publish("v1/devices/me/telemetry", buffer); 
}

// Task chuyên trách luồng giao tiếp Cloud (Task 6)
void task_push_mqtt(void *pvParameters) {
    EnvData_t receivedData;
    while (1) {
        mqtt_loop(); 

        if (pubSubClient1.connected() && qEnvDataMQTT != NULL) {
            // Lấy dữ liệu không block luồng (Delay = 0)
            if (xQueueReceive(qEnvDataMQTT, &receivedData, 0) == pdTRUE) {
                sendTelemetry(pubSubClient1, receivedData.temperature, receivedData.humidity);
                Serial.print("."); 
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Yield cho các task khác chạy
    }
}