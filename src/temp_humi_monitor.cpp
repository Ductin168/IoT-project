#include "temp_humi_monitor.h"
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);
//QueueHandle_t xQueue = xQueueCreate(5, sizeof(SensorData));
#define COLD 10.0
#define COOL 20.0
#define WARM 26.0
#define HOT 35.0

#define CRITICAL_DRY 30.0
#define DRY 40.0
#define HUMID 60.0
#define CRITICAL_HUMID 80.0
int status_temp;
int status_humi;
// 0: Bình thường, 1: Cảnh báo, 2: Nguy cấp

void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();

    //xQueue = (QueueHandle_t)pvParameters;
    //xQueue = xQueueCreate(5, sizeof(SensorData));
    SystemResources *res = (SystemResources *)pvParameters;
    SensorData data;
    
    lcd.begin();
    lcd.backlight();


    // Biến phụ để theo dõi trạng thái đèn nền khi nhấp nháy
    //bool isTextVisible = true;
    // Thời gian chờ của Queue (thay đổi tùy trạng thái)
    //TickType_t xTimeout = portMAX_DELAY;
    //vTaskDelay(pdMS_TO_TICKS(10));
    
    while (1){
        /* code */
        
        dht20.read();
        data.temp = dht20.getTemperature();
        data.humid = dht20.getHumidity();


        // Check if any reads failed and exit early
        if (isnan(data.temp) || isnan(data.humid)) {
            Serial.println("Failed to read from DHT sensor!");
            data.temp = data.humid =  -1;
            //return;
        }
        else
        {
            //task 1
            if (data.temp >= HOT || data.temp < COLD) status_temp = 2;
            else if (data.temp > WARM || data.temp < COOL) status_temp = 1;
            else status_temp = 0;

            //task 2
            if (data.humid >= CRITICAL_HUMID || data.humid < CRITICAL_DRY) status_humi = 2;
            else if (data.humid >= HUMID || data.humid < DRY) status_humi = 1;
            else  status_humi = 0;

            //task 3
            lcd.display();
            //isTextVisible = true;
            lcd.clear();
            lcd.setCursor(0, 1);

            if (status_temp == 2 || status_humi == 2) 
            {
                lcd.print("STATE: CRITICAL!");
                //xTimeout = pdMS_TO_TICKS(300);
            }
            else if (status_temp == 1 || status_humi == 1) 
            {
                lcd.print("STATE: WARNING");
                lcd.blink(); // 
                //xTimeout = portMAX_DELAY;
            }
            else 
            {
                lcd.print("STATE: NORMAL");
                lcd.noBlink(); // 
                //xTimeout = portMAX_DELAY;
            }
            lcd.setCursor(0, 0);
            //lcd.printf("T:", data.temp);
            //lcd.printf("T:%.1f %cC H:%.1f%%", data.temp, 223, data.humid);
            lcd.print("T:");
            lcd.print(data.temp, 1); // ,1 để lấy 1 chữ số thập phân
            lcd.write(223);          // In ký tự độ C (mã 223)
            lcd.print("C H:");
            lcd.print(data.humid, 1);
            lcd.print("%");
            //lcd.printf("T:%.1f C H:%.1f%%", data.temp, data.humid);
            // if (data.status == 2) {
            //     if (isTextVisible) lcd.noDisplay();
            //     else lcd.display();
            //     isTextVisible = !isTextVisible;
            // }

            // semaphore or queue
            if (res->xSensorQueue != NULL) xQueueSend(res->xSensorQueue, &data, pdMS_TO_TICKS(100));
            if (res->xLedSem != NULL) xQueueSend(res->xLedSem, &status_humi, pdMS_TO_TICKS(100));
            if (res->xNeoSem != NULL) xQueueSend(res->xNeoSem, &status_humi, pdMS_TO_TICKS(100));

            //xSemaphoreGive(res->xLedSem);
            //xSemaphoreGive(res->xNeoSem);
            //xSemaphoreGive(xBinarySemaphoreInternet);
        }

        // Print the results
        Serial.print("H: ");
        Serial.print(data.humid);
        Serial.print("% | T: ");
        Serial.print(data.temp);
        Serial.println("°C");

        //vTaskDelay(5000);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}

// void lcd_display_task(void *pvParameters)
// {
//     //xQueue = (QueueHandle_t)pvParameters;
//     SystemResources *res = (SystemResources *)pvParameters;
//     SensorData receivedData;
    
    
// }