#include "io.hh"

#include <hardware/gpio.h>
#include <pico/time.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

#include <cstdio>

#include "io.pio.h"

#define WR   14
#define IORQ 15
#define WAIT 16
#define A0    0
#define A1    1
#define D0    2
#define DATA_MASK (((uint32_t) 0xff) << 2)

namespace io {

volatile bool iorq_detected = false;
volatile bool wr = false;

static PIO pio = pio0;
static uint sm = 0;

void io_pio_irq()
{
    static uint32_t i = 0;
    printf("IRQ! %lu\n", i++);
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

    // setup IRQ
    irq_set_exclusive_handler(PIO0_IRQ_0, io_pio_irq);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio0_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS;
}

static uint8_t get_data(uint32_t pins)
{
    return (pins >> 2) & 0xff;
}

static void set_data(uint8_t data)
{
    gpio_set_dir_out_masked(DATA_MASK);
    gpio_put_masked(DATA_MASK, ((uint32_t) data) << 2);
}

static void release_data()
{
    // gpio_put_masked(DATA_MASK, 0);
    gpio_set_dir_in_masked(DATA_MASK);
}

void loop()
{
    volatile uint8_t data = 0x66;

    for (;;) {
        pio_sm_get_blocking(pio, sm);  // wait until signal received from PIO (IRQ low, WAIT low)

        uint32_t pins = gpio_get_all();
        printf("DATA: %02X\n", get_data(pins));
        printf("PINS: %08lX\n", pins);

        if ((pins & (1 << WR)) == 0) {  // write operation
            data = get_data(pins);
            printf("data set as 0x%02X\n", data);

        } else {  // WR is up (read operation)
            set_data(data + 1);
            printf("returning data as 0x%02X\n", data);
        }

        pio_sm_put(pio, sm, 0); // command executed, hand control back to PIO

        pio_sm_get_blocking(pio, sm);  // wait until signal received from PIO (IRQ high)
        release_data();
    }
}

/*
void loop()
{
    while (true) {
        sleep_ms(1000);
    }

    static volatile uint8_t data = 0x0;

    while (true) {

        if (iorq_detected) {

            printf("IORQ detected\n");

            // TODO - sleep ?

            printf("IORQ: %d  WR: %d\n", gpio_get(IORQ), wr);

            uint32_t pins = gpio_get_all();
            printf("DATA: %02X\n", get_data(pins));
            printf("PINS: %04X\n", pins);

            if (wr) {  // write operation
                data = get_data(pins);
                printf("data set as 0x%02X\n", data);

            } else {  // WR is up (read operation)
                set_data(data);
                printf("returning data as 0x%02X\n", data);
            }
            gpio_put(WAIT, 1);
            // printf("WAIT released\n");

            while (gpio_get(IORQ) == 0);  // wait until IORQ is released
            release_data();
            // printf("IORQ released\n");
            // printf("--------------\n");

            iorq_detected = false;
        }
    }
}
     */

}