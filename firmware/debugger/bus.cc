#include "bus.hh"

#include <avr/io.h>
#include <avr/cpufunc.h>

#define SET_DIR(port, pin, v) { if (v) DDR ## port |= (1 << pin); else DDR ## port &= ~(1 << pin); }
#define SET_PIN(port, pin, v) { if (v) PORT ## port |= (1 << pin); else PORT ## port &= ~(1 << pin); }
#define GET_PIN(port, pin) ((PIN ## port & (1 << pin)) ? 1 : 0)

namespace bus {

void init()
{
    SET_DIR(C, 0, 1)   // nmi = PC0
    SET_DIR(D, 7, 1)   // rst = PD7
    SET_DIR(L, 2, 1)   // clk = PL2
    SET_DIR(L, 7, 1)   // busrq = PL7

    set_busrq(1);
    set_nmi(1);
    set_rst(0);          // put Z80 in reset mode

    release_mem();       // set memory pins as pull up

    SET_PIN(L, 2, 0);    // clock initial position = 0
}

void set_nmi(bool v)
{
    // nmi = PC0
    SET_DIR(C, 0, 1)
    SET_PIN(C, 0, v)
    if (v != 0) SET_DIR(C, 0, 0)
}

void set_busrq(bool v)
{
    // busrq = PL7
    SET_DIR(L, 7, 1)
    SET_PIN(L, 7, v)
    if (v != 0) SET_DIR(L, 7, 0)
}

void set_rst(bool v)
{
    // rst = PD7
    SET_DIR(D, 7, 1)
    SET_PIN(D, 7, v)
    if (v != 0) SET_DIR(D, 7, 1)
}

bool get_busak()
{
    // busak = PL5
    return GET_PIN(L, 5);
}

bool get_m1()
{
    // m1 = PC1
    return GET_PIN(C, 1);
}

bool get_wait()
{
    // wait = PG1
    return GET_PIN(G, 1);
}

bool get_iorq()
{
    // iorq = PL3
    return GET_PIN(L, 3);
}

void pulse_clk()
{
    // clk = PL2
    SET_PIN(L, 2, 1)
    _NOP();
    SET_PIN(L, 2, 0)
}

void release_clk()
{
    SET_DIR(L, 2, 0)
}

MemPins get_mem()
{
    // wr = PH1
    // rd = PD3
    // mreq = PD1
    return {
        (bool) GET_PIN(H, 1),
        (bool) GET_PIN(D, 3),
        (bool) GET_PIN(D, 1),
    };
}

void set_mem(MemPins mem)
{
    // wr = PH1
    SET_DIR(H, 1, 1)
    SET_PIN(H, 1, mem.wr)

    // rd = PD3
    SET_DIR(D, 3, 1)
    SET_PIN(D, 3, mem.rd)

    // mreq = PD1
    SET_DIR(D, 1, 1)
    SET_PIN(D, 1, mem.mreq)
}

void release_mem()
{
    // wr = PH1
    SET_DIR(H, 1, 0)
    SET_PIN(H, 1, 1)  // set as pull-up

    // rd = PD3
    SET_DIR(D, 3, 0)
    SET_PIN(D, 3, 1)  // pull-up

    // mreq = PD1
    SET_DIR(D, 1, 0)
    SET_PIN(D, 1, 1)  // pull-up
}

// TODO --------

uint8_t get_data()
{
    // D = PK
    return PINK;
}

void set_data(uint8_t data)
{
    // D = PK
    DDRK = 0xff;
    PORTK = data;
}

void release_data()
{
    // D = PK
    DDRK = 0;
}

uint16_t get_addr()
{
    // A0..7 = PA
    // A8..F = PF
    return (((uint16_t) PINF) << 8) | PINA;
}

void set_addr(uint16_t addr)
{
    // A0..7 = PA
    // A8..F = PF
    DDRF = 0xff;
    DDRA = 0xff;
    PORTF = addr >> 8;
    PORTA = addr & 0xff;
}

void release_addr()
{
    // A0..7 = PA
    // A8..F = PF
    DDRF = 0;
    DDRA = 0;
}

uint8_t get_addr_high()
{
    // A16 = PJ1
    // A17 = PB6
    // A18 = PB4
    uint8_t v = 0;
    if (GET_PIN(J, 1))
        v |= 0b1;
    if (GET_PIN(B, 6))
        v |= 0b10;
    if (GET_PIN(B, 4))
        v |= 0b100;
    return v;
}

}
