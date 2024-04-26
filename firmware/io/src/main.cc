#include "z80_iorq.hh"
#include "serial.hh"

int main()
{
    serial::init();

    z80_iorq::init();
    z80_iorq::loop();
}
