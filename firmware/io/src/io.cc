#include "io.hh"

#include <hardware/gpio.h>
#include <cstdio>

#define IORQ  1
#define WAIT 22
#define WR   27
#define A0    4
#define A1    7
#define D0   14

namespace io {

static const uint32_t DATA_MASK = 0b00000000000011111100000000000000;

void init()
{
    gpio_init(A0);
    gpio_init(A1);
    gpio_init(IORQ);
    gpio_init(WR);
    for (size_t i = 0; i < 8; ++i)
        gpio_init(D0 + 1);

    gpio_init(WAIT);
    gpio_set_dir(WAIT, GPIO_OUT);
    gpio_put(WAIT, 1);
}

static void set_data(uint8_t data)
{
    gpio_set_dir_out_masked(DATA_MASK);
    gpio_put_masked(DATA_MASK, ((uint32_t) data) << 14);
}

static void release_data()
{
    gpio_put_masked(DATA_MASK, 0);
    gpio_set_dir_in_masked(DATA_MASK);
}

void loop()
{
    while (true) {
        uint32_t pins = gpio_get_all();
        gpio_put(WAIT, 0);

        if ((pins & (1 << IORQ)) == 0) {
            printf("-> %08lX\n", pins);

            if ((pins & (1 << WR)) == 0) {  // write operation

            } else {  // WR is up (read operation)
                set_data(0x53);
            }
        }

        getchar();   // TODO - remove
        gpio_put(WAIT, 1);
        while (gpio_get(IORQ) == 0);  // wait until IORQ is released
        release_data();
    }
}

}