#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

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
#define PIN_CLK 11
#define PIN_LAT 12
#define PIN_OE 13

int main()
{
    stdio_init_all();

    sleep_ms(2000); // låt USB komma upp

    if (cyw43_arch_init())
    {
        printf("CYW43 init failed\n");
        return 1;
    }

    while (true)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        printf("LED ON\n");
        sleep_ms(2000);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        printf("LED OFF\n");
        sleep_ms(2000);
    }
}
