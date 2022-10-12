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

#define BUFF_LEN (256)

_Noreturn void speed_task(void *params) {
    (void) params;
    static char buff[BUFF_LEN];
    int idx = 0;
    while (true) {
        int c = uart_getc(UART_ID);
        if (c == 13 || c == 10) {
            buff[idx] = 0;
            if(idx > 0) {
                struct GpsInfo gpsInfo = parseNmea(buff);
                if(gpsInfo.parsed) {
                    char buffOut[100];
                    snprintf(buffOut,100,"Speed: %f\r\n", gpsInfo.speedKts * 1.852); //todo remove
                    puts(buffOut);//todo remove
                }
            }
            idx = 0;
        } else {
            buff[idx] = c;
            idx = (idx + 1) % BUFF_LEN;
        }
    }
}

/*------------- MAIN -------------*/
int main(void) {
    board_init();
    initUart();
    initLeds();

    ledTaskHandle = xTaskCreateStatic(led_task, "led", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                                      led_stack, &led_taskdef);
    xTaskCreateStatic(speed_task, "speed", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2,
                      speed_stack, &speed_taskdef);


    vTaskStartScheduler();

}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
