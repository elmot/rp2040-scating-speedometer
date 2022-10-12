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

#define CDC_STACK_SIZE      configMINIMAL_STACK_SIZE
StackType_t led_stack[CDC_STACK_SIZE];
StaticTask_t led_taskdef;

_Noreturn void led_task(void *params) {
    (void) params;
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
            idx = (idx + 98) % 99;
        }
        vTaskDelay(500);
        writeSpeed(speed);
        speed = (speed + 1) % 30;
    }
}


/*------------- MAIN -------------*/
int main(void) {
    board_init();
    initPanel();
    initUart();
    initLeds();

    (void) xTaskCreateStatic(led_task, "cdc", CDC_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, led_stack, &led_taskdef);

    vTaskStartScheduler();

}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
