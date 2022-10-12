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

StackType_t led_stack[configMINIMAL_STACK_SIZE];
StaticTask_t led_taskdef;
TaskHandle_t ledTaskHandle;

StackType_t speed_stack[configMINIMAL_STACK_SIZE];
StaticTask_t speed_taskdef;
TaskHandle_t speedTaskHandle;

_Noreturn void speed_task(void *params) {
    (void) params;
    xTaskNotify(ledTaskHandle, 36, eSetValueWithOverwrite);
    for (int speed = 0; true; speed = (speed + 1) % 30) {
        vTaskDelay(500);
        xTaskNotify(ledTaskHandle, speed, eSetValueWithOverwrite);
    }
}

_Noreturn void led_task(void *params) {
    (void) params;
    initPanel();
//    char buff[100];
    int idx = 0;
    while (1) {
//        int c = uart_getc(UART_ID);
//        if (c == 13) {
//            buff[idx] = 0;
//            puts(buff);
//            idx = 0;
//        } else {
//            buff[idx] = c;
//            idx = (idx + 98) % 99;
//        }
        uint32_t speed;
        xTaskNotifyWait(0, 0, &speed, 0xFFFFFFFF);
        writeSpeed(speed);
    }
}


/*------------- MAIN -------------*/
int main(void) {
    board_init();
    initUart();
    initLeds();

    ledTaskHandle = xTaskCreateStatic(led_task, "led", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                                      led_stack, &led_taskdef);
    speedTaskHandle = xTaskCreateStatic(speed_task, "speed", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                                        speed_stack, &speed_taskdef);


    vTaskStartScheduler();

}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
