#ifndef IO_HH_
#define IO_HH_

#include <cstdint>

namespace io {

uint8_t read(uint8_t port);
void    write(uint8_t port, uint8_t data);

}

#endif //IO_HH_
