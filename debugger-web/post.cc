#include "post.hh"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <avr/pgmspace.h>

#include "bus.hh"
#include "buffer.hh"
#include "io.hh"
#include "memory.hh"
#include "random.hh"

namespace post {

static bool test_memory(uint16_t min_memory, uint16_t max_memory, bool write)
{
    const size_t N_ADDR = 32;
    const size_t N_TESTS = 16;

    uint16_t addr[N_ADDR];
    uint8_t data[N_ADDR];

    for (size_t i = 0; i < N_ADDR; ++i) {
        do {
            addr[i] = random::nextw();
        } while (addr[i] < min_memory || addr[i] > max_memory);
    }

    for (size_t i = 0; i < N_ADDR; ++i) {
        if (write) {
            data[i] = random::nextb();
            if (!memory::set(addr[i], data[i])) {
                snprintf_P(buffer::error, ERROR_SZ, PSTR("write [%04X] = %02X item %d"), addr[i], data[i], i);
                return false;
            }
        } else {
            data[i] = memory::get(addr[i]);
        }
    }

    for (size_t i = 0; i < N_TESTS; ++i) {
        for (size_t j = 0; j < N_ADDR; ++j) {
            uint8_t value = memory::get(addr[j]);
            if (value != data[j]) {
                snprintf_P(buffer::error, ERROR_SZ, PSTR("verify [%04X] = %02X attempt %d item %d"), addr[j], data[j], i, j);
                return false;
            }
        }
    }

    return true;
}

static bool read_rom_memory()
{
    return test_memory(0x0, 0x1fff, false);
}

static bool read_shared_memory()
{
    return test_memory(0x2000, 0x3fff, false);
}

static bool read_high_memory()
{
    return test_memory(0x4000, 0xffff, false);
}

static bool write_rom_memory()
{
    return test_memory(0x0, 0x1fff, true);
}

static bool write_shared_memory()
{
    return test_memory(0x2000, 0x3fff, true);
}

static bool write_high_memory()
{
    return test_memory(0x4000, 0xffff, true);
}

static bool write_memory_banks()
{
    return false;  // TODO

    // This test write one byte into each memory bank at the same address.
    // It also tests if the low memory remains the same when the banks change.

    uint16_t low_addr, high_addr;
    do { high_addr = random::nextw(); } while (high_addr < 0x4000);
    do { low_addr = random::nextw(); } while (low_addr < 0x2000 || low_addr >= 0x4000);
    
    uint8_t fixed_data = random::nextb();
    uint8_t data[8];
    for (int i = 0; i < 8; ++i)
        data[i] = random::nextb();

    memory::set(low_addr, fixed_data);
    for (int i = 0; i < 8; ++i) {
        io::write(0, i);   // choose memory bank
        memory::set(high_addr, data[i]);
    }

    for (int i = 0; i < 8; ++i) {
        io::write(0, i);

        uint8_t value = memory::get(low_addr);
        if (value != fixed_data) {
            snprintf_P(buffer::error, ERROR_SZ, PSTR("low memory changed with bankswitch: [%04X]=(expected: %02X, found %02X)"), low_addr, fixed_data, value);
            goto fail;
        }

        value = memory::get(high_addr);
        if (value != data[i]) {
            snprintf_P(buffer::error, ERROR_SZ, PSTR("mismatching value in bank %d"), i);
            goto fail;
        }
    }

    return true;

fail:
    io::write(0, 0);
    return false;
}

static bool write_ramonly()
{
    return false; // TODO

    uint16_t addr;
    do { addr = random::nextw(); } while (addr >= 0x2000);

    uint8_t rom_data = random::nextb();
    uint8_t ram_data = random::nextb();
    
    io::write(0, 0b0);      // set ROM
    memory::set(addr, rom_data);
    io::write(0, 0b1000);   // set RAM
    memory::set(addr, ram_data);

    io::write(0, 0b0);      // set ROM
    uint8_t value = memory::get(addr);
    if (value != rom_data) {
        snprintf_P(buffer::error, ERROR_SZ, PSTR("ROM value changed with ROMONLY: [%04X]=(expected: %02X, found %02X)"), addr, rom_data, value);
        goto fail;
    }
    io::write(0, 0b1000);   // set RAM
    value = memory::get(addr);
    if (value != ram_data) {
        snprintf_P(buffer::error, ERROR_SZ, PSTR("RAM value changed with ROMONLY: [%04X]=(found: %02X, ROM data: %02X, RAM data: %02X)"), addr, value, rom_data, ram_data);
        goto fail;
    }

    return true;

fail:
    io::write(0, 0);
    return false;
}

size_t run_tests(uint32_t* results)
{
    size_t count = 0;
    *results = 0;
    
#define TEST(test) { (*results) <<= 1; (*results) |= (!test()) & 1; ++count; }

    bus::set_rst(0);

    TEST(read_rom_memory)
    TEST(read_shared_memory)
    TEST(read_high_memory)

    TEST(write_rom_memory)
    TEST(write_shared_memory)
    TEST(write_high_memory)

    TEST(write_memory_banks);
    TEST(write_ramonly)

#undef TEST

    return count;
}

}
