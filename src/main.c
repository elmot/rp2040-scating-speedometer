#include "main.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_RX_PIN 1

static inline void initUart() {
    uart_init(UART_ID, BAUD_RATE);
    uart_set_fifo_enabled(UART_ID, true);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

static inline void initLeds() {
    gpio_init(TINY2040_LED_R_PIN);
    gpio_set_dir(TINY2040_LED_R_PIN, GPIO_OUT);
    gpio_put(TINY2040_LED_R_PIN, true);

    gpio_init(TINY2040_LED_G_PIN);
    gpio_set_dir(TINY2040_LED_G_PIN, GPIO_OUT);
    gpio_put(TINY2040_LED_G_PIN, true);
    gpio_init(TINY2040_LED_B_PIN);
    gpio_set_dir(TINY2040_LED_B_PIN, GPIO_OUT);
    gpio_put(TINY2040_LED_B_PIN, false);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

/*------------- MAIN -------------*/
int main(void) {
    board_init();
    initPanel();
    initUart();
    initLeds();
//    put_pixel(COLOR_MODERATE);
    writeSpeed(36);
    char buff[100];
    int idx = 0;
    int speed = 0;
    while (1) {
        int c = uart_getc(UART_ID);
        if (c == 13) {
            buff[idx] = 0;
            puts(buff);
            idx = 0;
        } else {
            buff[idx] = c;
            idx = (idx + 1) % 99;
        }
        sleep_ms(500);
        writeSpeed(speed);
        speed = (speed + 1) % 30;
    }

}

#pragma clang diagnostic pop

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
