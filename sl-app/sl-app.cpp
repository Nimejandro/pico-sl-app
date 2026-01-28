#include "pico/stdlib.h"
#include "matrix.h"

int main()
{
    stdio_init_all();
    matrix_init();
    matrix_clear();

    while (true)
    {
        for (int y = 0; y < 32; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                uint8_t r = (x * 255) / 63;
                uint8_t g = (y * 255) / 31;
                uint8_t b = 128;
                matrix_set_pixel(x, y, r, g, b);
            }
        }

        matrix_show();
    }
}
