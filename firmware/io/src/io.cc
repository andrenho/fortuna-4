#include "io.hh"

#include <hardware/gpio.h>

#define IORQ  2
#define WAIT 22
#define WR   27
#define A0    4
#define A1    7
#define D0   14

namespace io {

void init()
{
    gpio_set_dir(A0, GPIO_IN);
    gpio_set_dir(A1, GPIO_IN);
    gpio_set_dir(IORQ, GPIO_IN);
    gpio_set_dir(WR, GPIO_IN);
    for (size_t i = 0; i < 8; ++i)
        gpio_set_dir(D0 + i, GPIO_IN);

    gpio_set_dir(WAIT, GPIO_OUT);
    gpio_put(WAIT, true);
}

static void set_data(uint8_t data)
{
    uint32_t all_pins = gpio_get_all();
    gpio_put_all(all_pins);
}

void loop()
{
    while (true) {
        uint32_t pins = gpio_get_all();
        gpio_put(WAIT, false);

        if ((pins & (1 << IORQ)) == 0) {

            if ((pins & (1 << WR)) == 0) {  // write operation

            } else {  // WR is up (read operation)
                set_data(0x53);
            }
        }

        gpio_put(WAIT, true);
        while (gpio_get(IORQ) == 0);  // wait until IORQ is released
    }
}

}