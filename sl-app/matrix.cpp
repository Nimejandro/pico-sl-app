#include "matrix.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "matrix.pio.h"

#define PIN_R1 2
#define PIN_G1 3
#define PIN_B1 6
#define PIN_R2 7
#define PIN_G2 8
#define PIN_B2 9
#define PIN_A 10
#define PIN_B 20
#define PIN_C 19
#define PIN_D 18
#define PIN_E 21
#define PIN_CLK 11
#define PIN_LAT 12
#define PIN_OE 13

static RGB framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];
static uint16_t bitplanes[BITPLANES][MATRIX_HEIGHT][MATRIX_WIDTH / 2];

static PIO pio;
static uint sm;
static int dma_chan;

static void set_row(uint8_t row)
{
    gpio_put(PIN_A, row & 0x01);
    gpio_put(PIN_B, (row >> 1) & 0x01);
    gpio_put(PIN_C, (row >> 2) & 0x01);
    gpio_put(PIN_D, (row >> 3) & 0x01);
    gpio_put(PIN_E, (row >> 4) & 0x01);
}

static void latch()
{
    gpio_put(PIN_LAT, 1);
    busy_wait_us(1);
    gpio_put(PIN_LAT, 0);
}

static void oe_enable(bool en)
{
    gpio_put(PIN_OE, en ? 0 : 1); // OE is active low
}

static void build_bitplanes()
{
    for (int b = 0; b < BITPLANES; b++)
    {
        for (int y = 0; y < MATRIX_HEIGHT / 2; y++)
        {
            for (int x = 0; x < MATRIX_WIDTH / 2; x++)
            {
                uint16_t word = 0;

                for (int bit = 0; bit < 2; bit++)
                {
                    int px = x * 2 + bit;

                    RGB p1 = framebuffer[y][px];      // top
                    RGB p2 = framebuffer[y + 16][px]; // bottom

                    uint16_t r1 = (p1.r >> b) & 1;
                    uint16_t g1 = (p1.g >> b) & 1;
                    uint16_t b1 = (p1.b >> b) & 1;

                    uint16_t r2 = (p2.r >> b) & 1;
                    uint16_t g2 = (p2.g >> b) & 1;
                    uint16_t b2 = (p2.b >> b) & 1;

                    word |= (r1 << (bit * 6 + 0));
                    word |= (g1 << (bit * 6 + 1));
                    word |= (b1 << (bit * 6 + 2));
                    word |= (r2 << (bit * 6 + 3));
                    word |= (g2 << (bit * 6 + 4));
                    word |= (b2 << (bit * 6 + 5));
                }

                bitplanes[b][y][x] = word;
            }
        }
    }
}

static void dma_isr()
{
    dma_hw->ints0 = 1u << dma_chan;
}

void matrix_init()
{
    gpio_init(PIN_CLK);
    gpio_init(PIN_LAT);
    gpio_init(PIN_OE);
    gpio_init(PIN_A);
    gpio_init(PIN_B);
    gpio_init(PIN_C);
    gpio_init(PIN_D);
    gpio_init(PIN_E);

    gpio_set_dir(PIN_CLK, GPIO_OUT);
    gpio_set_dir(PIN_LAT, GPIO_OUT);
    gpio_set_dir(PIN_OE, GPIO_OUT);
    gpio_set_dir(PIN_A, GPIO_OUT);
    gpio_set_dir(PIN_B, GPIO_OUT);
    gpio_set_dir(PIN_C, GPIO_OUT);
    gpio_set_dir(PIN_D, GPIO_OUT);
    gpio_set_dir(PIN_E, GPIO_OUT);

    oe_enable(false);

    pio = pio0;
    sm = pio_claim_unused_sm(pio, true);

    uint offset = pio_add_program(pio, &matrix_program);
    pio_sm_config c = matrix_program_get_default_config(offset);

    sm_config_set_out_pins(&c, PIN_R1, 6);
    sm_config_set_set_pins(&c, PIN_CLK, 1);

    pio_sm_set_consecutive_pindirs(pio, sm, PIN_R1, 6, true);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_CLK, 1, true);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_dreq(&cfg, pio_get_dreq(pio, sm, true));

    dma_channel_configure(
        dma_chan,
        &cfg,
        &pio->txf[sm],
        bitplanes[0][0],
        MATRIX_WIDTH / 2,
        false);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_isr);
    irq_set_enabled(DMA_IRQ_0, true);
}

void matrix_clear()
{
    for (int y = 0; y < MATRIX_HEIGHT; y++)
        for (int x = 0; x < MATRIX_WIDTH; x++)
            framebuffer[y][x] = {0, 0, 0};
}

void matrix_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
        return;
    framebuffer[y][x] = {r, g, b};
}

void matrix_show()
{
    build_bitplanes();

    for (int b = 0; b < BITPLANES; b++)
    {
        for (int row = 0; row < MATRIX_HEIGHT / 2; row++)
        {
            oe_enable(false);
            set_row(row);

            dma_channel_set_read_addr(dma_chan, bitplanes[b][row], true);
            dma_channel_set_trans_count(dma_chan, MATRIX_WIDTH / 2, true);
            dma_channel_wait_for_finish_blocking(dma_chan);
            latch();
            oe_enable(true);
            busy_wait_us(1 << b);
            oe_enable(false);
        }
    }
}
