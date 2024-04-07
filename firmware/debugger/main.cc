#include <stdio.h>

#include <avr/cpufunc.h>

#include "buffer.hh"
#include "bus.hh"
#include "comm.hh"
#include "memory.hh"
#include "uart.hh"
#include "random.hh"
#include "z80.hh"

int main()
{
    random::init();
    bus::init();
    z80::init();
    uart_init();

    z80::reset();

    for (;;)
        comm::listen();

    return 0;
}
