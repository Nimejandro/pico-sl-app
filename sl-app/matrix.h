#pragma once

#include <stdint.h>

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32
#define BITPLANES 8

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

void matrix_init();
void matrix_clear();
void matrix_set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void matrix_show();
