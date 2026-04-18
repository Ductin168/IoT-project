#include "tinyml.h"

#define TEMP_MIN 20.0f
#define TEMP_MAX 40.0f

#define HUMI_MIN 0.0f
#define HUMI_MAX 100.0f

float normalize(float x, float min, float max) {
    return (x - min) / (max - min);
}

typedef struct {
    float temp;
    float humi;
} SensorData;



namespace
{
    tflite::ErrorReporter *tf_error_reporter = nullptr;
    const tflite::Model *tf_model = nullptr; 
    tflite::MicroInterpreter *tf_interpreter = nullptr;
    TfLiteTensor *tf_input = nullptr;
    TfLiteTensor *tf_output = nullptr;
    
    constexpr int kTensorArenaSize = 16 * 1024; 
    uint8_t tensor_arena[kTensorArenaSize];
}

void setupTinyML()
{
    Serial.println(">>> TensorFlow Lite Initializing...");
    static tflite::MicroErrorReporter micro_error_reporter;
    tf_error_reporter = &micro_error_reporter;

    tf_model = tflite::GetModel(smart_env_model_tflitel); 
    
    if (tf_model->version() != TFLITE_SCHEMA_VERSION)
    {
        tf_error_reporter->Report("Model version mismatch!");
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        tf_model, resolver, tensor_arena, kTensorArenaSize, tf_error_reporter);
    tf_interpreter = &static_interpreter;

    if (tf_interpreter->AllocateTensors() != kTfLiteOk)
    {
        tf_error_reporter->Report("AllocateTensors() failed");
        return;
    }

    tf_input = tf_interpreter->input(0);
    tf_output = tf_interpreter->output(0);

    Serial.println(">>> TinyML Initialized Successfully!");
}

void tiny_ml_task(void *pvParameters)
{
    setupTinyML();
    SensorData data;

    while (1)
    {
        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY))
        {
            // chuẩn hóa
            tf_input->data.f[0] = normalize(data.temp, TEMP_MIN, TEMP_MAX);
            tf_input->data.f[1] = normalize(data.humi, HUMI_MIN, HUMI_MAX);

            // thực hiện suy luận
            unsigned long start = millis();

            if (tf_interpreter->Invoke() == kTfLiteOk)
            {
                unsigned long end = millis();
                Serial.print("Inference time: ");
                Serial.print(end - start);
                Serial.println(" ms");

                // dự đoán nhãn
                int predicted_label = 0;
                float max_val = tf_output->data.f[0];

                for (int i = 1; i < 3; i++)   
                {
                    if (tf_output->data.f[i] > max_val)
                    {
                        max_val = tf_output->data.f[i];
                        predicted_label = i;
                    }
                }

                // nhãn dự đoán theo quy tắc
                int rule_label = 0;

                if (data.temp > 30) rule_label = 1;
                else if (data.humi > 75) rule_label = 2;
                else rule_label = 0;

                // độ chính xác 
                static int total = 0;
                static int correct = 0;

                if (predicted_label == rule_label) correct++;
                total++;

                Serial.print("Accuracy: ");
                Serial.println((float)correct / total);

                Serial.println("===== RESULT =====");

                Serial.print("AI: ");
                Serial.print(predicted_label);

                Serial.print(" | Rule: ");
                Serial.println(rule_label);

                Serial.print("Confidence: ");
                Serial.println(max_val);

                switch (predicted_label)
                {
                    case 0: Serial.println("Normal"); break;
                    case 1: Serial.println("Hot → Turn on Fan"); break;
                    case 2: Serial.println("Humid → Warning"); break;
                }
            }
        }
    }
}