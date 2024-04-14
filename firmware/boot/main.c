#include <stdbool.h>
#include <stdio.h>

#include <avr/cpufunc.h>
#include <avr/io.h>
#include <util/delay.h>

#define CLK    PORTD5
#define MEMRQ  PORTD5
#define WR     PORTD6
#define CLKENA PINA0
#define RST    PORTA1

static const uint8_t binary[] = { 0x1, 0x2, 0x4, 0x5 };

static void set_memwrite(bool write)
{
    if (write)
        PORTD &= ~((1 << MEMRQ) | (1 << WR));
    else
        PORTD |= (1 << MEMRQ) | (1 << WR);
}

static void write_byte(uint8_t addr, uint8_t data)
{
    PORTD &= 0b11110000;    // clear ADDR
    PORTD |= addr & 0b1111; // set ADDR
    PORTB = data;           // set DATA
    set_memwrite(true);
    _NOP();
    set_memwrite(false);
    _NOP();
}

static void write_ram(void)
{
    for (uint8_t i = 0; i < (uint8_t) sizeof(binary); ++i)
        write_byte(i, binary[i]);
}

static void z80_reset(void)
{
    if (PINA & (1 << CLKENA)) {  // if 1, then clock is enabled and we just wait
        _delay_ms(1);
    } else {
        for (size_t i = 0; i < 100; ++i) {  // do 50 cycles with the CPU in reset
            DDRD |= (1 << CLK);
            DDRD &= ~(1 << CLK);
        }
    }
}

int main(void)
{
    DDRB = 0xff;      // DATA line: output
    DDRD = 0b110111;  // A0-A3, CLK, MREQ, WR: output
    DDRA = 0b10;      // RST: output, CLKENA: input

    if (PINA & (1 << CLKENA))   // if 0, then clock is disabled so we need to set CLK as output
        DDRD |= (1 << DDD4);

    set_memwrite(false);
    PORTA &= ~(1 << RST);   // RST = 0 (resetting)

    write_ram();
    z80_reset();

    DDRB = 0;
    DDRD = 0;
    DDRA = 0;

    PORTA |= (1 << RST);   // RST = 1 (not resetting)
    MCUCR |= (1 << SM0) | (1 << SM1);   // power down ATTINY

    return 0;
}