#include "neo_blinky.h"

void neo_blinky(void *pvParameters){
    SystemResources *res = (SystemResources *)pvParameters;
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    //strip.setBrightness(255);
    strip.show();
    int status;

    while(1) 
    {        
        if (xQueueReceive(res->xNeoSem, &status, portMAX_DELAY)) 
        {
            if (status == 2) strip.fill(strip.Color(50, 0, 100)); // tím
            else if (status == 1) strip.fill(strip.Color(0, 0, 150)); // blue đậm
            else strip.fill(strip.Color(0, 255, 0)); // Green
            strip.show();
        }
        //vTaskDelay(pdMS_TO_TICKS(1000));
        /*strip.setPixelColor(0, strip.Color(255, 0, 0)); // Set pixel 0 to red
        strip.show(); // Update the strip

        // Wait for 500 milliseconds
        vTaskDelay(500);

        // Set the pixel to off
        strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn pixel 0 off
        strip.show(); // Update the strip

        // Wait for another 500 milliseconds
        vTaskDelay(500);*/
    }
}