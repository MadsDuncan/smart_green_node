#include <esp_log.h>
#include <string.h>
#include <Task.h>

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

#include "sdkconfig.h"

#define USE_SEMAPHORE_BLE_OCCU 1

#ifdef USE_SEMAPHORE_BLE_OCCU
extern SemaphoreHandle_t semaphore_BLE_occu;
#endif
