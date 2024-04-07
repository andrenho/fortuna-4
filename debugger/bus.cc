#include "bus.hh"

#include <avr/io.h>
#include <avr/cpufunc.h>

#define SET_PIN(port, pin, v) { if (v) port |= _BV(pin); else port &= ~_BV(pin); }

namespace bus {

void init()
{
    DDRA = 0b00010111;   // rom_we, busrq, nmi, clk, clk_ena
    DDRJ = 0b00000010;   // rst

    set_rom_we(1);
    set_busrq(1);
    set_nmi(1);
    set_rst(0);          // put Z80 in reset mode

    release_mem();       // set memory pins as pull up

    PORTA &= ~_BV(PA4);  // clock initial position = 0
}

void set_rom_we(bool v) { SET_PIN(PORTA, PA0, v) }

void set_nmi(bool v)
{
    DDRA |= (1 << DDA2);
    SET_PIN(PORTA, PA2, v)
    if (v != 0) DDRA &= ~(1 << DDA2);
}

void set_busrq(bool v) {
    DDRA |= (1 << DDA1);
    SET_PIN(PORTA, PA1, v)
    if (v != 0) DDRA &= ~(1 << DDA1);
}

void set_rst(bool v) {
    DDRJ |= (1 << DDJ1);
    SET_PIN(PORTJ, PJ1, v)
    if (v != 0) DDRJ &= ~(1 << DDJ1);
}

bool get_busak()
{
    return PINL & _BV(PINL2);
}

bool get_m1()
{
    return PINL & _BV(PINL0);
}

bool get_int()
{
    return PINL & _BV(PINL5);
}

bool get_wait()
{
    return PINL & _BV(PINL3);
}

bool get_iorq()
{
    return PINL & _BV(PINL1);
}

bool get_ramonly()
{
    return PIND & _BV(PIND0);
}

void pulse_clk()
{
    PORTA |= _BV(PA4);
    _NOP();
    PORTA &= ~_BV(PA4);
}

void release_clk()
{
    DDRA &= ~0b1000;
}

void pulse_y0w()
{
    DDRJ |= 0b1;
    SET_PIN(PORTJ, PJ0, false);
    _NOP();
    SET_PIN(PORTJ, PJ0, true);
    DDRJ &= ~0b1;
}

MemPins get_mem()
{
    uint8_t a = PINA;
    return {
        (bool) (a & _BV(5)),
        (bool) (a & _BV(6)),
        (bool) (a & _BV(7))
    };
}

void set_mem(MemPins mem)
{
    DDRA |= 0b11100000;
    SET_PIN(PORTA, PA5, mem.wr);
    SET_PIN(PORTA, PA6, mem.rd);
    SET_PIN(PORTA, PA7, mem.mreq);
}

void release_mem()
{
    DDRA &= ~0b11100000;
    PORTA |= 0b11100000;   // set as pullups
}

uint8_t get_data()
{
    return PINC;
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
