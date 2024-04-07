#ifndef Z80_HH_
#define Z80_HH_

#include <stdint.h>

#include "bus.hh"

#define MAX_BKP 32
#define NO_BKP 0xffff

namespace z80 {

struct StepCycleStatus {
    uint8_t  data;
    uint16_t addr;
    bool     m1      : 1;
    bool     iorq    : 1;
    bool     busak   : 1;
    bool     wait    : 1;
    bool     int_    : 1;
    MemPins  mem_pins;
};

struct StepStatus {
    uint16_t af, bc, de, hl, afx, bcx, dex, hlx, ix, iy, sp, pc;
    uint16_t stack[8];
    uint8_t  bank;
    bool     ramonly;
    bool has_info;
};

struct LastOp {
    uint16_t pc;
    uint8_t  op;
};

bool is_present();
void release_bus();

void            init();
void            reset();
StepCycleStatus step_cycle();
LastOp          step();
StepStatus      step_nmi();
StepStatus      next();
LastOp          debug_run();

void             bkp_swap(uint16_t bkp);
uint16_t const*  bkp_list();

}

#endif
