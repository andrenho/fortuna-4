#include "io.hh"

#include "bus.hh"
#include "z80.hh"

namespace io {

void write(uint8_t pin, uint8_t value)
{
    if (z80::is_present())
        return;   // TODO
    
    if (pin != 0)
        return;   // TODO

    bus::set_data(value);
    bus::pulse_y0w();
    bus::release_data();
}

}
