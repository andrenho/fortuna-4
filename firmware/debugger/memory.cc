#include "memory.hh"

#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdio.h>

#include "bus.hh"
#include "z80.hh"

namespace memory {

bool set(uint16_t addr, uint8_t data)
{
    if (z80::is_present())
        z80::release_bus();

    // don't set if data is already there
    uint8_t current_data = get(addr);
    if (data == current_data)
        return true;

    // set pinout
    bus::set_addr(addr);
    bus::set_data(data);

    // set WE pin, and wait
    bus::set_mem({ 0, 1, 0 });
    _NOP();

    // release pins
    bus::release_mem();
    bus::release_addr();
    bus::set_data(~data);  // mess up DATA
    bus::release_data();

    return true;
}

uint8_t get(uint16_t addr)
{
    if (z80::is_present())
        z80::release_bus();

    bus::set_addr(addr);
    bus::set_mem({ 1, 0, 0 });
    _NOP();

    uint8_t data = bus::get_data();

    bus::release_addr();
    bus::release_mem();

    return data;
}

}
