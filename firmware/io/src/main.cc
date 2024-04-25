#include "pico/stdlib.h"

#include <stdio.h>

#include "io.hh"

int main()
{
    stdio_usb_init();

    /*
    io::init();
    io::loop();
     */

    for (;;);
}
