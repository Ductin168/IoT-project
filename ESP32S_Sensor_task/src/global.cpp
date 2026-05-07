#include "global.h"

String WIFI_SSID = "";
String WIFI_PASS = "";
String CORE_IOT_TOKEN = "";
String CORE_IOT_SERVER = "";
String CORE_IOT_PORT = "";

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();