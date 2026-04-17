#include "mqtt_client.h"
#include "global.h"
#include "task_webserver.h" // Để gọi được Webserver_sendata
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>

unsigned long last_mqtt_push = 0;
double device1_lat = 10.880018410410052;
double device1_long = 106.80633605864662;

WiFiClient mqttEspClient1;
PubSubClient pubSubClient1(mqttEspClient1);

void processRPC(PubSubClient& client, int dev_id, char* topic, byte* payload, unsigned int length) {
    Serial.print("Device "); Serial.print(dev_id);
    Serial.print(" Received ["); Serial.print(topic); Serial.print("]: ");
    
    String msg = "";
    for (int i = 0; i < length; i++) msg += (char)payload[i];
    Serial.println(msg);

    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) return;

    if (doc.containsKey("method") && dev_id == 1) {
        String methodName = doc["method"].as<String>();
        bool value = doc["params"]; 
        
        Serial.printf("⚡ CoreIoT ra lệnh: [%s] -> %s\n", methodName.c_str(), value ? "ON" : "OFF");
        
        // 1. Nếu là LED Task 1 cứng
        if (methodName == "led") {
            // ledBlinkEnabled = value; 
            digitalWrite(48, value ? HIGH : LOW);
            Webserver_sendata("{\"page\":\"device_status\",\"gpio\":48,\"status\":\"" + String(value ? "ON" : "OFF") + "\"}");
        } 
        // 2. NẾU LÀ RELAY ĐỘNG (Báo cho Web biết bằng TÊN)
        else {
            String stat = value ? "ON" : "OFF";
            // 👉 Lệnh cực quan trọng: Réo Web bằng Tên
            Webserver_sendata("{\"page\":\"cloud_update\",\"name\":\"" + methodName + "\",\"status\":\"" + stat + "\"}");
        }

        // Báo cáo ngược lên CoreIoT
        StaticJsonDocument<100> res;
        res[methodName.c_str()] = value; 
        char buffer[100];
        serializeJson(res, buffer);
        client.publish("v1/devices/me/attributes", buffer);
        
        Serial.println("☁️ Đã chốt trạng thái [" + methodName + "] lên Attributes CoreIoT!");
    }
}

void mqttCallback1(char* topic, byte* payload, unsigned int length) {
    processRPC(pubSubClient1, 1, topic, payload, length);
}

void mqtt_reconnect(PubSubClient& client, const char* c_id, const char* user, const char* pass) {
    if (!client.connected()) {
        Serial.print("Connecting MQTT...");
        if (client.connect(c_id, user, pass)) {
            Serial.println(" connected 🟢");
            client.subscribe("v1/devices/me/rpc/request/+");
        } else {
            Serial.print(" failed 🔴, rc="); 
            Serial.println(client.state());
        }
    }
}

void mqtt_setup() {
    pubSubClient1.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());
    pubSubClient1.setCallback(mqttCallback1);
}

void mqtt_loop() {
    if (WiFi.status() != WL_CONNECTED) return; 
    mqtt_reconnect(pubSubClient1, "ESP32_UNO", CORE_IOT_TOKEN.c_str(), "");
    pubSubClient1.loop();
}

void sendTelemetry(PubSubClient& client, const char* dev_name, int temp, int humi, int light, bool is_dev1) {
    if (!client.connected()) return;
    StaticJsonDocument<256> doc;
    doc["temperature"] = temp;
    doc["humidity"] = humi;
    doc["light"] = light; 

    if (is_dev1) {
        doc["latitude"] = device1_lat;
        doc["longitude"] = device1_long; 
    }
    char buffer[256];
    serializeJson(doc, buffer);
    client.publish("v1/devices/me/telemetry", buffer);
}

void pushRealTelemetry(uint32_t delay_ms) {
    //  sendTelemetry(pubSubClient1, "Dev1", (int)glob_temperature, (int)glob_humidity, glob_light, true);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

void task_push_mqtt(void *pvParameters) {
    while (1) {
        mqtt_loop(); 
        if (millis() - last_mqtt_push >= 3000) {
            last_mqtt_push = millis();
            // sendTelemetry(pubSubClient1, "Dev1", (int)glob_temperature, (int)glob_humidity, glob_light, true);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}