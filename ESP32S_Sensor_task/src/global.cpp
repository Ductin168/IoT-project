#include "global.h"

// Chỉ giữ lại Semaphore để đồng bộ hóa nếu cần, các chuỗi cấu hình đã bị xóa bỏ
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();