#include "bus.hh"

#include <avr/io.h>
#include <avr/cpufunc.h>

#define SET_PIN(port, pin, v) { if (v) port |= _BV(pin); else port &= ~_BV(pin); }
#define SET_DIR SET_PIN
#define GET_PIN(port, pin) ((port & _BV(pin)) ? 1 : 0)

namespace bus {

void init()
{
    SET_DIR(DDRC, DDC3, 1)   // NMI
    SET_DIR(DDRK, DDK0, 1)   // CLKENA
    SET_DIR(DDRK, DDK6, 1)   // BUSRQ
    SET_DIR(DDRF, DDF4, 1)   // RST
    SET_DIR(DDRF, DDF6, 1)   // CLK

    set_busrq(1);
    set_nmi(1);
    set_rst(0);          // put Z80 in reset mode

    release_mem();       // set memory pins as pull up

    PORTF &= ~(1 << PF6);  // clock initial position = 0
}

void set_nmi(bool v)
{
    // nmi = PC3
    SET_DIR(DDRC, DDC3, 1)
    SET_PIN(PORTC, PC3, v)
    if (v != 0) SET_DIR(DDRC, DDC3, 0)
}

void set_busrq(bool v)
{
    // busrq = PK6
    SET_DIR(DDRK, DDK6, 1)
    SET_PIN(PORTK, PK6, v)
    if (v != 0) SET_DIR(DDRK, DDK6, 0)
}

void set_rst(bool v)
{
    // rst = PF4
    SET_DIR(DDRF, DDF4, 1)
    SET_PIN(PORTF, PF4, v)
    if (v != 0) SET_DIR(DDRF, DDF4, 0)
}

bool get_busak()
{
    // busak = PC5
    return GET_PIN(PINC, PINC5);
}

bool get_m1()
{
    // m1 = PK2
    return GET_PIN(PINK, PINK2);
}

bool get_wait()
{
    // wait = PC1
    return GET_PIN(PINC, PINC1);
}

bool get_iorq()
{
    // iorq = PK4
    return GET_PIN(PINK, PINK4);
}

void pulse_clk()
{
    // clk = PF6
    SET_PIN(PORTA, PA4, 1);
    _NOP();
    SET_PIN(PORTA, PA4, 0);
}

void release_clk()
{
    SET_DIR(DDRF, DDF6, 0);
}

MemPins get_mem()
{
    // wr = PF3
    // rd = PK7
    // mreq = PC4
    return {
        (bool) GET_PIN(PINF, PINF3),
        (bool) GET_PIN(PINK, PINK7),
        (bool) GET_PIN(PINC, PINC4),
    };
}

void set_mem(MemPins mem)
{
    // wr = PF3
    SET_DIR(DDRF, DDF3, 1);
    SET_PIN(PORTF, PORTF3, mem.wr);

    // rd = PK7
    SET_DIR(DDRK, DDK7, 1);
    SET_PIN(PORTK, PORTK3, mem.rd);

    // mreq = PC4
    SET_DIR(DDRC, DDC4, 1);
    SET_PIN(PORTC, PORTC4, mem.mreq);
}

void release_mem()
{
    // wr = PF3
    SET_DIR(DDRF, DDF3, 0);
    SET_PIN(PORTF, PORTF3, 1);  // set as pull-up

    // rd = PK7
    SET_DIR(DDRK, DDK7, 0);
    SET_PIN(PORTK, PORTK3, 1);  // pull-up

    // mreq = PC4
    SET_DIR(DDRC, DDC4, 0);
    SET_PIN(PORTC, PORTC4, 1);   // pull-up
}

// TODO --------

uint8_t get_data()
{
    // D[0..1] = PA[0..1], D[2..7] = PL[2..7]
    return (PINA & 0b11) | (PINL & 0b11111100);
}

void set_data(uint8_t data)
{
    DDRC = 0xff;
    PORTC = data;
}

void release_data()
{
    DDRC = 0;
}

uint16_t get_addr()
{
    return (((uint16_t) PINK) << 8) | PINF;
}

void set_addr(uint16_t addr)
{
    DDRK = 0xff;
    DDRF = 0xff;
    PORTK = addr >> 8;
    PORTF = addr & 0xff;
}

void release_addr()
{
    DDRK = 0;
    DDRF = 0;
}

uint8_t get_addr_high()
{
    uint8_t v = 0;
    if (PINB & _BV(PINB4))
        v |= 0b1;
    if (PINB & _BV(PINB5))
        v |= 0b10;
    if (PINB & _BV(PINB6))
        v |= 0b100;
    return v;
}

}
