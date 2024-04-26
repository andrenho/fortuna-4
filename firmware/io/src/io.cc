#include "io.hh"

#include <cstdio>

namespace io {

static volatile uint8_t sanity_data = 0x0;

uint8_t read(uint8_t port)
{
    // printf("Read (port %d)\n", port);
    uint8_t data;
    switch (port) {
        case 0x1:
            data = sanity_data + 1;
            break;
    }
    return data;
}

void write(uint8_t port, uint8_t data)
{
    // printf("Write (port %d, data 0x%02X)\n", port, data);
    switch (port) {
        case 0x1:
            sanity_data = data;
            break;
    }
}

}