#ifndef SERIAL_HH_
#define SERIAL_HH_

#include <cstdint>

namespace serial {

void    init();
uint8_t read();
void    write(uint8_t data);

}

#endif //SERIAL_HH_
