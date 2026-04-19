#include "led_blinky.h"

void led_blinky(void *pvParameters){
  pinMode(LED_GPIO, OUTPUT); // LED_GPIO = 48
  SystemResources *res = (SystemResources *)pvParameters;
  int status = 0; // 0: Sáng đứng, 1: Nháy chậm, 2: Nháy nhanh
  //xQueueReceive(res->LedSem, &data, 0);
  while(1) 
  {
    TickType_t xTicksToWait = (status == 0) ? portMAX_DELAY : 0;
    if (xQueueReceive(res->xLedSem, &status, xTicksToWait))
    {  

    }
    if (status == 2) 
    {
      digitalWrite(LED_GPIO, !digitalRead(LED_GPIO));
      vTaskDelay(pdMS_TO_TICKS(100));
    } 
    else if (status == 1) 
    {
      // Nháy chậm kiểu nhịp thở/chớp tắt
      digitalWrite(LED_GPIO, !digitalRead(LED_GPIO));
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    else 
    {
      // Sáng liên tục
      digitalWrite(LED_GPIO, HIGH);
    }
    



    /*
    digitalWrite(LED_GPIO, HIGH);  // turn the LED ON
    vTaskDelay(1000);
    digitalWrite(LED_GPIO, LOW);  // turn the LED OFF
    vTaskDelay(1000);
    */
  }
}