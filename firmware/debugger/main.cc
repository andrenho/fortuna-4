#include <util/delay.h>
#include "bus.hh"
#include "comm.hh"
#include "uart.hh"
#include "random.hh"
#include "z80.hh"

int main()
{
    _delay_ms(100);

    random::init();
    bus::init();
    z80::init();
    uart_init();

    z80::reset();

    for (;;)
        comm::listen();
}
