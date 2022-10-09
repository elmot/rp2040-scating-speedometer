/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>

#include "bsp/board.h"
#include "generated/ws2812.pio.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  { //GRB
  COLOR_WAIT_SATELLITE = 0x008060,
  COLOR_WAIT_POSITION = 0x608000,
  COLOR_STALL = 0x1010FF,
  COLOR_SLOW = 0xFF0000,
  COLOR_MODERATE = 0x40FF00,
  COLOR_FAST = 0x08FF08,
};
#define WS2812_STATE_MACHINE 0
#define WS2812_PIO pio0

void cdc_task(void);

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  uint offset = pio_add_program(WS2812_PIO, &ws2812_program);
  ws2812_program_init(WS2812_PIO, WS2812_STATE_MACHINE, offset, 26, 800000, false);
  put_pixel(COLOR_MODERATE);
  while (1)
  {
  }

}
#pragma clang diagnostic pop

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
