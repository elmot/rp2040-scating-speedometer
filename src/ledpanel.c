#include "main.h"
#include "hardware/dma.h"
#include "generated/ws2812.pio.h"

#define WS2812_STATE_MACHINE 0
#define WS2812_PIO pio0
#define WS2812_PIN 26

#define DMA_CHANNEL 0
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)

#define CHAR_WIDTH 8
#define KMH_WIDTH 13
static const uint8_t font8x8_0_9[10][CHAR_WIDTH] =
        {{
                 0b01111110,
                 0b11111111,
                 0b10000001,
                 0b10000001,
                 0b10000001,
                 0b10000001,
                 0b11111111,
                 0b01111110
         },
         {
                 0b00000000,
                 0b00010000,
                 0b00100000,
                 0b01000000,
                 0b11111111,
                 0b11111111,
                 0b00000000,
                 0b00000000
         },
         {
                 0b01000011,
                 0b11000111,
                 0b10001101,
                 0b10001001,
                 0b10001001,
                 0b10011001,
                 0b11110001,
                 0b01100001
         },
         {
                 0b01000010,
                 0b10000001,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b11111111,
                 0b01101110
         },
         {
                 0b00001000,
                 0b00011000,
                 0b00111000,
                 0b01101000,
                 0b11001000,
                 0b11111111,
                 0b11111111,
                 0b00001000
         },
         {
                 0b11110010,
                 0b11110011,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b10011111,
                 0b10001110
         },
         {
                 0b01111110,
                 0b11111111,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b11011111,
                 0b01001110
         },
         {
                 0b10000000,
                 0b10000011,
                 0b10000111,
                 0b10001100,
                 0b10011000,
                 0b10110000,
                 0b11100000,
                 0b11000000
         },
         {
                 0b01101110,
                 0b11111111,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b11111111,
                 0b01101110
         },
         {
                 0b01100010,
                 0b11110011,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b10010001,
                 0b11111111,
                 0b01111110
         }};

const uint8_t kmh_sign[KMH_WIDTH] = {
        0b01111111,
        0b00001100,
        0b00010011,
        0b00000000,
        0b00011111,
        0b00010000,
        0b00011111,
        0b00010000,
        0b00001111,
        0b00000000,
        0b01111111,
        0b00010000,
        0b00001111
};
enum { //GRB
    COLOR_WAIT_SATELLITE = 0x008060,
    COLOR_WAIT_POSITION = 0x608000,
    COLOR_STALL = 0x1010FF,
    COLOR_SLOW = 0xFF0000,
    COLOR_SLOW_DARK = 0x600000,
    COLOR_MODERATE = 0x40FF00,
    COLOR_MODERATE_DARK = 0x103F00,
    COLOR_FAST = 0x08FF08,
    COLOR_FAST_DARK = 0x023F02,
};

#define PIX_BUFFER_SIZE (8 * 32)
static uint32_t pixBuffer[PIX_BUFFER_SIZE];
static uint_fast16_t pixelIndex = 0;


//todo verify all and use
void initDma() {
    dma_claim_mask(DMA_CHANNEL_MASK);

    dma_channel_config channel_config = dma_channel_get_default_config(DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(WS2812_PIO, WS2812_STATE_MACHINE, true));
    channel_config_set_irq_quiet(&channel_config, true);
    dma_channel_configure(DMA_CHANNEL,
                          &channel_config,
                          &WS2812_PIO->txf[WS2812_STATE_MACHINE],
                          pixBuffer,
                          PIX_BUFFER_SIZE,
                          false);
}



static inline void clearFrame() {
    memset(pixBuffer, 0, PIX_BUFFER_SIZE * sizeof (uint32_t));
    pixelIndex = 9;
}

static inline void showFrame() {
    sleep_ms(10);
    dma_channel_transfer_from_buffer_now(DMA_CHANNEL, pixBuffer, PIX_BUFFER_SIZE);
}

/* todo uncomment
static inline void savePixel(int x, int y, uint32_t color) {
    int idx =
}

*/
static inline void put_pixel(uint32_t pixel_grb) {
    pixBuffer[pixelIndex++] = pixel_grb << 8;
}



void initPanel() {
    uint offset = pio_add_program(WS2812_PIO, &ws2812_program);
    ws2812_program_init(WS2812_PIO, WS2812_STATE_MACHINE, offset, WS2812_PIN, 800000, false);
    clearFrame();
    initDma();
    showFrame();
}

static bool oddColumn = false;//todo remove
static inline void writeBlankColumn() {
    for (int i = 0; i < 8; ++i) {
        put_pixel(0);//todo DMA
    }
    oddColumn = !oddColumn;
}

static inline void writeColumn(uint8_t data, uint32_t color) {
    for (int i = 0; i < 8; ++i) {
        bool pixValue;
        if (oddColumn) {
            pixValue = data & (1 << i);
        } else {
            pixValue = data & (1 << (7 - i));
        }
        put_pixel(pixValue ? color : 0); //todo DMA
    }
    oddColumn = !oddColumn;
}



static void writeDigits(uint8_t leadDigit, uint8_t tailDigit, uint32_t color, uint32_t kmhColor) {
    oddColumn = false;
    if (leadDigit == 0) {
        for (int i = 0; i < CHAR_WIDTH; ++i) {
            writeBlankColumn();
        }
    } else {
        for (int i = 0; i < CHAR_WIDTH; ++i) {
            writeColumn(font8x8_0_9[leadDigit][i], color);
        }
    }
    writeBlankColumn();
    for (int i = 0; i < CHAR_WIDTH; ++i) {
        writeColumn(font8x8_0_9[tailDigit][i], color);
    }
    writeBlankColumn();
    writeBlankColumn();
    for (int i = 0; i < KMH_WIDTH; ++i) {
        writeColumn(kmh_sign[i], kmhColor);
    }
}

void writeSpeed(unsigned int kmh) {
    kmh = 10;
    clearFrame();
    if (kmh < 3) {
        for (int i = 0; i < (256 + 36) / 37; ++i) {
            for (int j = 0; j < 36; ++j) {
                put_pixel(0); //todo DMA
            }
            put_pixel(COLOR_STALL);//todo DMA
        }
    } else {
        uint32_t color = COLOR_FAST;
        uint32_t kmhColor = COLOR_FAST_DARK;
        if (kmh < 9) {
            color = COLOR_SLOW;
            kmhColor = COLOR_SLOW_DARK;
        } else if (kmh < 18) {
            color = COLOR_MODERATE;
            kmhColor = COLOR_MODERATE_DARK;
        }
        writeDigits((kmh / 10) % 10, kmh % 10, color, kmhColor);
    }
    showFrame();
}

_Noreturn void led_task(void *params) {
    (void) params;
    initPanel();
    while (1) {
        xTaskNotifyWait(0, 0, NULL, 0xFFFFFFFF);
        if(xSemaphoreTake(gpsDataMutex, 5000)) {

            writeSpeed(gpsDataXchange.speedKts * 10.0 /*remove*/ + 0.5 );
            xSemaphoreGive(gpsDataMutex);
        }
    }
}
