#ifndef EMULATOR_FORTUNA_H
#define EMULATOR_FORTUNA_H

#include <stddef.h>
#include <stdint.h>

#include "z80/Z80.h"

void       fortuna_init();
void       fortuna_reset();
Z80 const* fortuna_step();
void       fortuna_write_to_rom(uint16_t addr, int* values, size_t n_values);

#endif //EMULATOR_FORTUNA_H
