#ifndef BUFFER_HH_
#define BUFFER_HH_

#include <stdint.h>

#define INPUT_SZ 256
#define ERROR_SZ 128
#define DATA_SZ 64

namespace buffer {

extern char input[];
extern char error[];
extern uint8_t data[];

}

#endif
