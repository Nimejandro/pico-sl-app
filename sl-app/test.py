from hub75 import Hub75Driver, row_addressing
from machine import Pin

WIDTH = 64
HEIGHT = 32

driver = Hub75Driver(
    row_addressing=row_addressing.Binary(
        base_pin=Pin(9),
        bit_count=4
    ),
    shift_register_depth=64,
    base_data_pin=Pin(0),
    base_clock_pin=Pin(6),
    output_enable_pin=Pin(8),
)

buffer = bytearray(WIDTH * HEIGHT * 3)

def set_pixel(x, y, r, g, b):
    i = (y * WIDTH + x) * 3
    buffer[i] = r
    buffer[i+1] = g
    buffer[i+2] = b

set_pixel(10, 10, 255, 0, 0)

driver.load_rgb888(buffer)
driver.flip()