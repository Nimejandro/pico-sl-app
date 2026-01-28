#include <stdio.h>
#include "pico/stdlib.h"

// Pin definitions for the Waveshare 64x32 RGB LED matrix
#define PIN_R1 2   // Red channel for top half
#define PIN_G1 3   // Green channel for top half
#define PIN_B1 6   // Blue channel for top half
#define PIN_R2 7   // Red channel for bottom half
#define PIN_G2 8   // Green channel for bottom half
#define PIN_B2 9   // Blue channel for bottom half
#define PIN_A 10   // Row select bit 0
#define PIN_B 20   // Row select bit 1
#define PIN_C 19   // Row select bit 2
#define PIN_D 18   // Row select bit 3
#define PIN_CLK 11 // Clock signal for shifting data
#define PIN_LAT 12 // Latch signal to display data
#define PIN_OE 13  // Output enable signal

// Initialize a GPIO pin as output
void gpio_out(int pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

// Set the row select pins based on the row number (0-15)
void set_row(uint8_t row)
{
    gpio_put(PIN_A, row & 0x01);
    gpio_put(PIN_B, row & 0x02);
    gpio_put(PIN_C, row & 0x04);
    gpio_put(PIN_D, row & 0x08);
}

int main()
{
    stdio_init_all(); // Initialize standard I/O for debugging

    // Array of all pins used for the LED matrix
    int pins[] = {
        PIN_R1, PIN_G1, PIN_B1,
        PIN_R2, PIN_G2, PIN_B2,
        PIN_A, PIN_B, PIN_C, PIN_D,
        PIN_CLK, PIN_LAT, PIN_OE};

    // Initialize all pins as outputs and set them low
    for (int i = 0; i < sizeof(pins) / sizeof(int); i++)
    {
        gpio_out(pins[i]);
        gpio_put(pins[i], 0);
    }

    // Set initial states: OE high (output disabled), LAT and CLK low
    gpio_put(PIN_OE, 1);
    gpio_put(PIN_LAT, 0);
    gpio_put(PIN_CLK, 0);

    // Variables for pixel position
    int x = 0;     // Column position (0-63)
    int y = 0;     // Row position (0-31)
    int color = 0; // Color index: 0=red, 1=green, 2=blue

    // Main loop to animate a moving pixel cycling through colors
    while (true)
    {

        // Set row
        uint8_t row = y & 0x0F; // 0..15 (only 16 rows per half)
        set_row(row);

        gpio_put(PIN_OE, 1); // blank while shifting

        // shift 64 pixels (columns)
        for (int col = 0; col < 64; col++)
        {
            // Determine if the pixel at this position should be on
            bool top_pixel_on = (y < 16) && (col == x);     // Top half (rows 0-15)
            bool bottom_pixel_on = (y >= 16) && (col == x); // Bottom half (rows 16-31)

            // Set color data based on current color index
            bool r_top = (color == 0) && top_pixel_on;
            bool g_top = (color == 1) && top_pixel_on;
            bool b_top = (color == 2) && top_pixel_on;
            bool r_bottom = (color == 0) && bottom_pixel_on;
            bool g_bottom = (color == 1) && bottom_pixel_on;
            bool b_bottom = (color == 2) && bottom_pixel_on;

            // Set color data for top half
            gpio_put(PIN_R1, r_top ? 1 : 0);
            gpio_put(PIN_G1, g_top ? 1 : 0);
            gpio_put(PIN_B1, b_top ? 1 : 0);

            // Set color data for bottom half
            gpio_put(PIN_R2, r_bottom ? 1 : 0);
            gpio_put(PIN_G2, g_bottom ? 1 : 0);
            gpio_put(PIN_B2, b_bottom ? 1 : 0);

            // clock pulse to shift data
            gpio_put(PIN_CLK, 1);
            sleep_us(1);
            gpio_put(PIN_CLK, 0);
        }

        // latch the data to display
        gpio_put(PIN_LAT, 1);
        sleep_us(1);
        gpio_put(PIN_LAT, 0);

        // enable output to show the row
        gpio_put(PIN_OE, 0);

        // show row a bit longer
        sleep_ms(500);

        // move pixel to next position
        x++;
        if (x >= 64)
        {
            x = 0;
            y++;
            if (y >= 32)
            {
                y = 0;
            }
        }
        // Cycle to next color
        color = (color + 1) % 3;
    }
}