#ifndef MEMORY_HH_
#define MEMORY_HH_

#include <stdint.h>

namespace memory {

bool    set(uint16_t addr, uint8_t data);
uint8_t get(uint16_t addr);

void    set_memory_state(uint8_t high_addr, bool ramonly);

}

#endif
