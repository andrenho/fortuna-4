#include "z80_iorq.hh"

#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

#include "io.hh"

#include "z80_iorq.pio.h"

#define WR   14
#define IORQ 15
#define WAIT 16
#define A0    0
#define A1    1
#define D0    2
#define DATA_MASK (((uint32_t) 0xff) << 2)

#include <cstdio>

namespace z80_iorq {

volatile bool iorq_detected = false;
volatile bool wr = false;

static PIO pio = pio0;
static uint sm = 0;

static void __not_in_flash_func(set_data)(uint8_t data)
{
    gpio_set_dir_out_masked(DATA_MASK);
    gpio_put_masked(DATA_MASK, ((uint32_t) data) << 2);
}

static void __not_in_flash_func(release_data)()
{
    gpio_put_masked(DATA_MASK, 0);
    gpio_set_dir_in_masked(DATA_MASK);
}

void init()
{
    // initialize pins
    gpio_init(A0);
    gpio_init(A1);
    gpio_init(WR);
    gpio_init_mask(DATA_MASK);

    // initialize PIO I/O program
    sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &io_program);

    pio_sm_config c = io_program_get_default_config(offset);
    pio_gpio_init(pio, WAIT);
    sm_config_set_set_pins(&c, WAIT, 1);
    sm_config_set_in_pins(&c, IORQ);
    pio_sm_set_consecutive_pindirs(pio, sm, WAIT, 1, true);
    sm_config_set_clkdiv(&c, 1.0f);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    /*
    irq_set_exclusive_handler(PIO0_IRQ_0, &pio_irq);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio0_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS;
    */
}

void __not_in_flash_func(loop)()
{
    for (;;) {
        pio_sm_get_blocking(pio, sm);  // wait until signal received from PIO (IRQ low, WAIT low)

        // printf("X");

        uint32_t pins = gpio_get_all();
        uint8_t addr = pins & 0b11;

        if ((pins & (1 << WR)) == 0) {  // write operation
            uint8_t data = (pins >> 2) & 0xff;
            io::write(addr, data);

        } else {  // WR is up (read operation)
            set_data(io::read(addr));
        }

        pio_sm_put(pio, sm, 0); // command executed, hand control back to PIO

        pio_sm_get_blocking(pio, sm);  // wait until signal received from PIO (IRQ high)
        release_data();
    }
}

}