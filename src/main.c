#include "main.h"
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
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

SemaphoreHandle_t gpsDataMutex;
StaticSemaphore_t gpsDataMutexBuffer;

/*------------- MAIN -------------*/
int main(void) {
    board_init();
    initUart();
    initLeds();
    gpsDataMutex = xSemaphoreCreateMutexStatic( &gpsDataMutexBuffer );

    configASSERT( gpsDataMutex );

    ledTaskHandle = xTaskCreateStatic(led_task, "led", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                                      led_stack, &led_taskdef);
    xTaskCreateStatic(speed_task, "speed", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                      speed_stack, &speed_taskdef);


    vTaskStartScheduler();

}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
