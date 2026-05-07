#ifndef __NEO_BLINKY__
#define __NEO_BLINKY__
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>



// Định nghĩa chân GPIO kết nối với LED NeoPixel (RGB) trên bo mạch Yolo UNO
#define NEO_PIN 45

// Số lượng LED NeoPixel được sử dụng
#define LED_COUNT 1 

// Task RTOS thực hiện Task 2: 
// Đọc dữ liệu độ ẩm từ Queue và thay đổi màu sắc của LED NeoPixel tương ứng với 3 mức độ ẩm.
// Task này sử dụng Semaphore để đồng bộ hóa việc hiển thị.
void neo_blinky(void *pvParameters);
#endif