#include "random.hh"

#include <avr/eeprom.h>

#define EEPROM_POS ((uint16_t *) 0x0)

namespace random {

static uint16_t seed = 0;

void init()
{
    seed = eeprom_read_word(EEPROM_POS);
}

uint8_t nextb()
{
    return nextw() & 0xff;
}

uint16_t nextw()
{
   seed = (seed * 1664525U + 1013904223U) & 0xFFFF;
   return seed;
}

void write_to_eeprom()
{
    eeprom_write_word(EEPROM_POS, seed);
}

}
