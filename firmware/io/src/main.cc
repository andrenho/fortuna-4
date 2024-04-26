#include "pico/stdlib.h"

#include <stdio.h>

#include "z80_iorq.hh"

int main()
{
    stdio_usb_init();

    z80_iorq::init();
    z80_iorq::loop();
}
