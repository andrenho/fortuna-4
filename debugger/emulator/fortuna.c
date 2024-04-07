#include "fortuna.h"

#include <stdbool.h>
#include <stdint.h>

#include "z80/Z80.h"

static uint8_t rom[0x2000];
static uint8_t ram[512 * 1024];
static uint8_t bank = 0;
static bool    ramonly = false;
static Z80 z80;

void fortuna_init()
{
}

void fortuna_reset()
{
    bank = 0;
    ramonly = false;
    ResetZ80(&z80);
}

Z80 const* fortuna_step()
{
    RunZ80(&z80);
    return &z80;
}

void fortuna_write_to_rom(uint16_t addr, int* values, size_t n_values)
{
    for (size_t i = 0; i < n_values; ++i) {
        WrZ80(addr + i, values[i]);
        if (!ramonly && i < 0x2000)
            rom[addr + i] = (uint8_t) (values[i]);
    }
}

byte InZ80(register word Port)
{
    return 0;
}

void OutZ80(register word Port,register byte Value)
{
}

void PatchZ80(register Z80 *R)
{
}

word LoopZ80(register Z80 *R)
{
    return INT_QUIT;
}

static uint32_t translate_addr(uint16_t addr)
{
    if (ramonly)
        return addr;
    else
        return addr + (bank * 0x10000);
}

void WrZ80(register word Addr,register byte Value)
{
    if (!ramonly && Addr < 0x2000)  // block writing to ROM
        return;

    uint32_t a = translate_addr(Addr);
    ram[a] = Value;
}

byte RdZ80(register word Addr)
{
    if (!ramonly && Addr < 0x2000)
        return rom[Addr];
    else
        return ram[translate_addr(Addr)];
}
