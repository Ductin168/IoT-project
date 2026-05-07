#include "global.h"

// Khởi tạo các Queue (Hàng đợi) dùng để truyền dữ liệu môi trường (EnvData_t) giữa các Task
QueueHandle_t qEnvData = NULL;      // Hàng đợi gốc (ít dùng trực tiếp nếu đã chia các queue con)
QueueHandle_t qLedCommand = NULL;   // Hàng đợi nhận lệnh điều khiển LED từ Web/MQTT xuống thiết bị
QueueHandle_t qEnvDataAI = NULL;    // Hàng đợi truyền dữ liệu cho Task chạy TinyML (Task 5)
QueueHandle_t qEnvDataMQTT = NULL;  // Hàng đợi truyền dữ liệu cho Task gửi MQTT (Task 6)
QueueHandle_t qEnvDataLCD = NULL;   // Hàng đợi truyền dữ liệu cho Task hiển thị LCD/OLED (Task 3)

// Khởi tạo các Semaphore/Mutex dùng để đồng bộ hóa và bảo vệ tài nguyên
SemaphoreHandle_t xSemaphoreLED = NULL; // Binary Semaphore bảo vệ quyền điều khiển LED
QueueHandle_t qEnvDataLED = NULL;       // Hàng đợi truyền dữ liệu cho Task điều khiển LED đơn (Task 1)
QueueHandle_t qEnvDataNeo = NULL;       // Hàng đợi truyền dữ liệu cho Task điều khiển NeoPixel (Task 2)
SemaphoreHandle_t xSemaphoreNeo = NULL; // Binary Semaphore bảo vệ quyền điều khiển NeoPixel

// Semaphore đặc biệt đã được cấp phát sẵn bằng xSemaphoreCreateBinary() 
// dùng để báo hiệu khi có kết nối Internet (chuyển từ trạng thái mất mạng sang có mạng)
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

// Mutex bảo vệ tài nguyên kết nối WiFi, đảm bảo không có 2 task cùng can thiệp mạng đồng thời
SemaphoreHandle_t wifiMutex = NULL;