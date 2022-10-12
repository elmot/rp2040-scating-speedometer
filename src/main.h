//
// Created by elmot on 10-10-22.
//

#ifndef RP2040_SKATING_SPEEDOMETER_MAIN_H
#define RP2040_SKATING_SPEEDOMETER_MAIN_H
#include "FreeRTOS.h"
#include "task.h"
#include "bsp/board.h"
#include "hardware/uart.h"
#include "hardware/pio.h"

void initPanel();
_Noreturn void led_task(void *params);

struct GpsInfo {
    bool parsed;
    bool validTime;
    bool valid;
    uint8_t hour;
    uint8_t minute;
    double speedKts;
    double lat;
    double lon;
};

struct GpsInfo parseNmea(char* buf);

#endif //RP2040_SKATING_SPEEDOMETER_MAIN_H
