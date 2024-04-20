#include <util/delay.h>
#include "bus.hh"
#include "comm.hh"
#include "uart.hh"
#include "random.hh"
#include "z80.hh"

int main()
{
    uart_init();
    random::init();

    _delay_ms(200);

    bus::init();
    z80::init();

    z80::reset();

    for (;;)
        comm::listen();
}
