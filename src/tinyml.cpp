#include "tinyml.h"

#define TEMP_MIN 20.0f
#define TEMP_MAX 40.0f

#define HUMI_MIN 0.0f
#define HUMI_MAX 100.0f

float normalize(float x, float min, float max)
{
    return (x - min) / (max - min);
}

typedef struct
{
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

    tf_model = tflite::GetModel(smart_env_model_tflite);

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

            unsigned long start = millis();

            if (tf_interpreter->Invoke() == kTfLiteOk)
            {
                unsigned long end = millis();
                Serial.print("Inference time: ");
                Serial.print(end - start);
                Serial.println(" ms");

                // ===== OUTPUT =====
                float temp_score = tf_output->data.f[0];
                float humi_score = tf_output->data.f[1];

                int temp_pred = temp_score > 0.5f ? 1 : 0;
                int humi_pred = humi_score > 0.5f ? 1 : 0;

                // ===== RULE (GROUND TRUTH) =====
                int temp_rule = (data.temp >= 30) ? 1 : 0;
                int humi_rule = (data.humi >= 75) ? 1 : 0;

                // ===== ACCURACY =====
                static int total = 0;
                static int correct = 0;

                if (temp_pred == temp_rule && humi_pred == humi_rule)
                    correct++;

                total++;

                Serial.print("Accuracy: ");
                Serial.println((float)correct / total);

                // ===== LOG =====
                Serial.println("===== RESULT =====");

                Serial.print("Temp Score: ");
                Serial.print(temp_score);
                Serial.print(" → ");
                Serial.println(temp_pred ? "HOT" : "NORMAL");

                Serial.print("Humi Score: ");
                Serial.print(humi_score);
                Serial.print(" → ");
                Serial.println(humi_pred ? "HIGH" : "NORMAL");

                Serial.print("Rule Temp: ");
                Serial.println(temp_rule);

                Serial.print("Rule Humi: ");
                Serial.println(humi_rule);

                if (temp_pred == 1)
                {
                    Serial.println("Turn ON Fan");
                }

                if (humi_pred == 1)
                {
                    Serial.println("Humidity Warning");
                }

                if (temp_pred == 0 && humi_pred == 0)
                {
                    Serial.println("Environment Normal");
                }
            }
        }
    }
}