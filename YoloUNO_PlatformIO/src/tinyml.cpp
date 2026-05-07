#include "tinyml.h"
#include "global.h"           
#include "task_webserver.h"   
#include <ArduinoJson.h>      

// Ngưỡng min/max dùng để chuẩn hóa dữ liệu đầu vào (Normalize) cho mô hình TinyML
#define TEMP_MIN 20.0f
#define TEMP_MAX 40.0f
#define HUMI_MIN 0.0f
#define HUMI_MAX 100.0f
// Hàm chuẩn hóa giá trị về khoảng [0, 1] trước khi đưa vào mô hình (Yêu cầu bắt buộc của TFLite)
float normalize(float x, float min, float max) {
    float val = (x - min) / (max - min);
    if (val < 0.0f) return 0.0f;
    if (val > 1.0f) return 1.0f;
    return val;
}

namespace {

    // Khai báo các đối tượng con trỏ của TensorFlow Lite

    tflite::ErrorReporter *tf_error_reporter = nullptr;
    const tflite::Model *tf_model = nullptr; 
    tflite::MicroInterpreter *tf_interpreter = nullptr;
    TfLiteTensor *tf_input = nullptr;
    TfLiteTensor *tf_output = nullptr;

    // Cấp phát vùng nhớ tĩnh (Tensor Arena) cho mô hình hoạt động. 
    // Kích thước 16KB là đủ cho mô hình Dense nhỏ của bài toán này.
    constexpr int kTensorArenaSize = 16 * 1024; 
    uint8_t tensor_arena[kTensorArenaSize];
}

void setupTinyML() {
    Serial.println(">>> TensorFlow Lite Initializing...");
    static tflite::MicroErrorReporter micro_error_reporter;
    tf_error_reporter = &micro_error_reporter;

    // Load mô hình từ mảng byte tĩnh trong dht_anomaly_model.h
    tf_model = tflite::GetModel(smart_env_model_tflite); 
    
    // Kiểm tra tính tương thích phiên bản của model
    if (tf_model->version() != TFLITE_SCHEMA_VERSION) {
        tf_error_reporter->Report("Model version mismatch!");
        return;
    }

    // Nạp tất cả các toán tử (Ops) và khởi tạo Interpreter
    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        tf_model, resolver, tensor_arena, kTensorArenaSize, tf_error_reporter);
    tf_interpreter = &static_interpreter;

    // Phân bổ bộ nhớ cho các tensor đầu vào/đầu ra
    if (tf_interpreter->AllocateTensors() != kTfLiteOk) {
        tf_error_reporter->Report("AllocateTensors() failed");
        return;
    }

    tf_input = tf_interpreter->input(0);
    tf_output = tf_interpreter->output(0);
    Serial.println(">>> TinyML Initialized Successfully!");
}

// Task 5: Triển khai mô hình AI để đánh giá độ bất thường của môi trường
void tiny_ml_task(void *pvParameters) {
    setupTinyML();
    EnvData_t data;

    while (1) {
        // Đợi nhận dữ liệu mới từ Queue (chờ vô hạn bằng portMAX_DELAY)
        if (qEnvDataAI != NULL && xQueueReceive(qEnvDataAI, &data, portMAX_DELAY) == pdTRUE) {
            
            float t = data.temperature;
            float h = data.humidity;

            // Truyền dữ liệu đã chuẩn hóa vào Tensor Input
            tf_input->data.f[0] = normalize(t, TEMP_MIN, TEMP_MAX);
            tf_input->data.f[1] = normalize(h, HUMI_MIN, HUMI_MAX);

            // Chạy suy luận (Inference)
            if (tf_interpreter->Invoke() == kTfLiteOk) {
                // Lấy kết quả từ Tensor Output
                float temp_score = tf_output->data.f[0];
                float humi_score = tf_output->data.f[1];

                // Phân loại kết quả (Threshold = 0.5)
                int temp_pred = temp_score > 0.5f ? 1 : 0;
                int humi_pred = humi_score > 0.5f ? 1 : 0;

                String status_text = "";
                int ui_label = 0; 

                // Xử lý logic gán nhãn trạng thái dựa trên kết quả AI
                if (temp_pred == 1 && humi_pred == 1) {
                    status_text = "NGUY HIỂM: NÓNG & ẨM CAO!";
                    ui_label = 3; 
                } else if (temp_pred == 1) {
                    status_text = "Cảnh báo: Nóng - Bật Quạt";
                    ui_label = 1;
                } else if (humi_pred == 1) {
                    status_text = "Cảnh báo: Ẩm Cao";
                    ui_label = 2;
                } else {
                    status_text = "Môi trường Bình thường";
                    ui_label = 0;
                }

                Serial.printf("🤖 [AI RESULT] T: %.1f, H: %.1f -> %s\n", t, h, status_text.c_str());

                // Đóng gói dữ liệu AI thành JSON để đẩy lên Web Interface (Websocket)
                StaticJsonDocument<256> aiDoc;
                aiDoc["page"] = "home";
                aiDoc["ai_label"] = ui_label;
                aiDoc["ai_status"] = status_text;
                aiDoc["ai_score_temp"] = temp_score; 
                aiDoc["ai_score_humi"] = humi_score; 
                aiDoc["is_anomaly"] = (temp_pred == 1 || humi_pred == 1); 

                String jsonOutput;
                serializeJson(aiDoc, jsonOutput);
                Webserver_sendata(jsonOutput);
            }
        }
    }
}