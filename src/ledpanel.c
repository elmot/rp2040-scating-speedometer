#include "main.h"
#include "hardware/dma.h"
#include "generated/ws2812.pio.h"

#define WS2812_STATE_MACHINE 0
#define WS2812_PIO pio0
#define WS2812_PIN 26

#define DMA_CHANNEL 0
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)

#define WAVE_SIGN_INDEX (10)

const uint8_t font8x8_0_9[11][8] =
        {{
                 0b01111110,
                 0b11000011,
                 0b11000011,
                 0b11000011,
                 0b11000011,
                 0b11000011,
                 0b11000011,
                 0b01111110,
         },
         {
                 0b00001100,
                 0b00011100,
                 0b00101100,
                 0b01001100,
                 0b00001100,
                 0b00001100,
                 0b00001100,
                 0b00001100,
         },
         {
                 0b01111110,
                 0b11000011,
                 0b00000011,
                 0b00000110,
                 0b00111100,
                 0b01100000,
                 0b11000000,
                 0b11111111,
         },
         {
                 0b01111110,
                 0b10000011,
                 0b00000011,
                 0b00111110,
                 0b00000011,
                 0b00000011,
                 0b10000011,
                 0b01111110,
         },
         {
                 0b00001110,
                 0b00011110,
                 0b00110110,
                 0b01100110,
                 0b11111111,
                 0b00000110,
                 0b00000110,
                 0b00000110,
         },
         {
                 0b11111111,
                 0b11000000,
                 0b11000000,
                 0b11111110,
                 0b00000011,
                 0b00000011,
                 0b11000011,
                 0b01111110,
         },
         {
                 0b01111110,
                 0b11000011,
                 0b11000000,
                 0b11111110,
                 0b11000011,
                 0b11000011,
                 0b11000011,
                 0b01111110,
         },
         {
                 0b11111111,
                 0b00000011,
                 0b00000110,
                 0b00001100,
                 0b00011000,
                 0b00110000,
                 0b01100000,
                 0b01100000,
         },
         {
                 0b01111110,
                 0b11000011,
                 0b11000011,
                 0b01111110,
                 0b11000011,
                 0b11000011,
                 0b11000011,
                 0b01111110,
         },
         {
                 0b01111110,
                 0b11000011,
                 0b11000011,
                 0b01111111,
                 0b00000011,
                 0b00000011,
                 0b11000011,
                 0b01111110,
         },
         {
                 0b00110000,
                 0b01000000,
                 0b10000000,
                 0b10000000,
                 0b10000000,
                 0b10000000,
                 0b01000000,
                 0b0011000,
         }};

const uint32_t kmh13x8[8] = {
        0b0000000000000,
        0b1000000000100,
        0b1000000000100,
        0b1010111100110,
        0b1100101010101,
        0b1100101010101,
        0b1010101010101,
        0b1010101010101,
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
static int waveIndex = 0;
static TickType_t waveIndexLastChanged = 0;

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
    memset(pixBuffer, 0, PIX_BUFFER_SIZE * sizeof(uint32_t));
}

static inline void showFrame() {
    sleep_ms(10);
    dma_channel_transfer_from_buffer_now(DMA_CHANNEL, pixBuffer, PIX_BUFFER_SIZE);
}

static inline void savePixel(int x, int y, uint32_t color) {
    unsigned int idx;
    if (x & 1) {
        idx = 7 + (x * 8) - y;
    } else {
        idx = x * 8 + y;
    }
    assert(idx < PIX_BUFFER_SIZE);
    if (idx < PIX_BUFFER_SIZE) {
        pixBuffer[idx] = color << 8;
    }
}


void initPanel() {
    uint offset = pio_add_program(WS2812_PIO, &ws2812_program);
    ws2812_program_init(WS2812_PIO, WS2812_STATE_MACHINE, offset, WS2812_PIN, 800000, false);
    clearFrame();
    initDma();
    showFrame();
}

static void writeDigit(uint8_t x, uint8_t digit, uint32_t color) {
    for (int i = 0, mask = 0x80; i < 8; ++i, mask >>= 1) {
        for (int j = 0; j < 8; ++j) {
            if (font8x8_0_9[digit][j] & mask)
                savePixel(x + i, j, color);
        }
    }
}

void writeSpeed(unsigned int kmh) {
    if (kmh < 3) {
        savePixel(7, 4, COLOR_STALL);
        savePixel(11, 2, COLOR_STALL);
        savePixel(15, 6, COLOR_STALL);
        savePixel(20, 1, COLOR_STALL);
        savePixel(24, 7, COLOR_STALL);
        savePixel(28, 3, COLOR_STALL);
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
        clearFrame();
        if ((kmh / 10) % 10 > 0) {
            writeDigit(0, (kmh / 10) % 10, color);
        }
        writeDigit(9, kmh % 10, color);
        for (int i = 0, mask = 0x1000; i < 14; ++i, mask >>= 1) {
            for (int j = 0; j < 8; ++j) {
                if (kmh13x8[j] & mask)
                    savePixel(18 + i, j, kmhColor);
            }
        }
    }
}

_Noreturn void led_task(void *params) {
    (void) params;
    initPanel();
    while (1) {
        xTaskNotifyWait(0, 0, NULL, 0xFFFFFFFF);
        if (xSemaphoreTake(gpsDataMutex, 5000)) {
            clearFrame();
            if (gpsDataXchange.valid) {
                writeSpeed(lround(gpsDataXchange.speedKts * 1.852));
            } else {
                TickType_t time = xTaskGetTickCount();
                if (time - waveIndexLastChanged > 250) {
                    waveIndex = (waveIndex + 7) % 24;
                    waveIndexLastChanged = time;
                }
                uint32_t color = gpsDataXchange.validTime ? COLOR_WAIT_POSITION : COLOR_WAIT_SATELLITE;
                writeDigit(waveIndex, WAVE_SIGN_INDEX, color);
            }
            xSemaphoreGive(gpsDataMutex);
            showFrame();
        }
    }
}
