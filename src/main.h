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

void writeSpeed(unsigned int kmh);
void initPanel();

#endif //RP2040_SKATING_SPEEDOMETER_MAIN_H
