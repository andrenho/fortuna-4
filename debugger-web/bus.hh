#ifndef BUS_HH_
#define BUS_HH_

#include <stdint.h>

struct MemPins {
    bool wr   : 1;
    bool rd   : 1;
    bool mreq : 1;
};

namespace bus {

void     init();

void     set_rom_we(bool v);
void     set_nmi(bool v);
void     set_busrq(bool v);

void     set_rst(bool v);

bool     get_busak();
bool     get_m1();
bool     get_int();
bool     get_wait();
bool     get_iorq();
bool     get_ramonly();

void     pulse_clk();
void     release_clk();

void     pulse_y0w();

MemPins  get_mem();
void     set_mem(MemPins mem);
void     release_mem();

uint8_t  get_data();
void     set_data(uint8_t data);
void     release_data();

uint16_t get_addr();
void     set_addr(uint16_t addr);
void     release_addr();

uint8_t  get_addr_high();

}

#endif
