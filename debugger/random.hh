#ifndef RANDOM_HH_
#define RANDOM_HH_

#include <stdint.h>

namespace random {

void     init();
uint8_t  nextb();
uint16_t nextw();
void     write_to_eeprom();

}

#endif
