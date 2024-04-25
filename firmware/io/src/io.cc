#include "io.hh"

#include <hardware/gpio.h>
#include <cstdio>
#include <pico/time.h>

#define IORQ  1
#define WAIT 22
#define WR   27
#define A0    4
#define A1    7
#define D0   14

namespace io {

static const uint32_t DATA_MASK = 0b00000000001111111100000000000000;

volatile uint32_t pins_on_iorq = 0;  // 0 = no iorq

static void iorq_callback(uint gpio, uint32_t event_mask)
{
    pins_on_iorq = gpio_get_all();
    gpio_put(WAIT, 0);
}

void init()
{
    gpio_init(A0);
    gpio_init(A1);
    gpio_init(WR);

    gpio_init_mask(DATA_MASK);

    gpio_init(WAIT);
    gpio_set_dir(WAIT, GPIO_OUT);
    gpio_put(WAIT, 1);

    gpio_init(IORQ);
    gpio_set_irq_enabled_with_callback(IORQ, GPIO_IRQ_EDGE_FALL, true, iorq_callback);
}

static uint8_t get_data(uint32_t pins)
{
    return (pins >> 14) & 0xff;
}

static void set_data(uint8_t data)
{
    gpio_set_dir_out_masked(DATA_MASK);
    gpio_put_masked(DATA_MASK, ((uint32_t) data) << 14);
}

static void release_data()
{
    // gpio_put_masked(DATA_MASK, 0);
    gpio_set_dir_in_masked(DATA_MASK);
}

void loop()
{
    volatile uint8_t data = 0x0;

    while (true) {
        if (pins_on_iorq != 0) {

            printf("-> %08lX\n", pins_on_iorq);

            if ((pins_on_iorq & (1 << WR)) == 0) {  // write operation
                data = get_data(pins_on_iorq);
                printf("data set as 0x%02X\n", data);

            } else {  // WR is up (read operation)
                set_data(data);
                printf("returning data as 0x%02X\n", data);
            }
            gpio_put(WAIT, 1);

            while (gpio_get(IORQ) == 0);  // wait until IORQ is released
            release_data();

            pins_on_iorq = 0;
        }
    }
}

}