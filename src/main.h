//
// Created by elmot on 10-10-22.
//

#ifndef RP2040_SKATING_SPEEDOMETER_MAIN_H
#define RP2040_SKATING_SPEEDOMETER_MAIN_H
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "bsp/board.h"
#include "hardware/uart.h"
#include "hardware/pio.h"

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_RX_PIN 1

void initPanel();
_Noreturn void led_task(void *params);
_Noreturn void speed_task(void *params);

volatile struct GpsData_t {
    bool parsed;
    bool validTime;
    bool valid;
    uint8_t hour;
    uint8_t minute;
    double speedKts;
    double lat;
    double lon;
};

extern SemaphoreHandle_t gpsDataMutex;
extern volatile struct GpsData_t gpsDataXchange;
extern TaskHandle_t ledTaskHandle;


#endif //RP2040_SKATING_SPEEDOMETER_MAIN_H
