#include "comm.hh"

#include <stdio.h>

#include <avr/pgmspace.h>

#include "buffer.hh"
#include "memory.hh"
#include "post.hh"
#include "z80.hh"

#define NO_VALUE 0x80000000

namespace comm {

static bool input_buffer()
{
    int i = 0;

    for (;;) {
        char c = getchar();
        if (c == 10 || c == 13) {
            buffer::input[i++] = 0;
            putchar('\n');
            return true;
        } else if ((c == 8 || c == 127)) {
            if (i > 0) {
                --i;
                printf_P(PSTR("\b \b"));
            }
            continue;
        }
        buffer::input[i++] = c;
        putchar(c);
        if (i > INPUT_SZ)
            return false;
    }
}

static uint32_t next_value(size_t* i)
{
    if (buffer::input[*i] == 0)
        return NO_VALUE;

    int value = 0;

    for (;;) {
        char c = buffer::input[(*i)++];
        if (c == 0 || c == 32) {
            if (c == 0)
                --(*i); // keep index on top of EOS
            return value;
        }
        value <<= 4;
        if (c >= '0' && c <= '9')
            value |= (c - '0');
        else if (c >= 'A' && c <= 'F')
            value |= (c - 'A' + 0xa);
        else if (c >= 'a' && c <= 'f')
            value |= (c - 'a' + 0xa);
        else
            return NO_VALUE;
    }
}

static bool post_tests()
{
    uint32_t results;
    size_t n_tests = post::run_tests(&results);
    putchar(results == 0 ? '+' : '-');
    putchar(' ');

    for (size_t i = 0; i < n_tests; ++i) {
        putchar(((results >> (n_tests - i - 1)) & 1) ? '-' : '+');
        putchar(i + 'a');
        putchar(' ');
    }

    putchar('\n');

    return true;
}

static bool read_memory(size_t i)
{
    uint32_t addr = next_value(&i);
    if (addr == NO_VALUE)
       return false;

    uint32_t count = next_value(&i);
    if (count == NO_VALUE)
        return false;

    printf_P(PSTR("+ %X"), count);

    for (size_t j = 0; j < count; ++j)
        printf_P(PSTR(" %02X"), memory::get(addr + j));
    putchar('\n');

    return true;
}

static bool write_memory(size_t i)
{
    uint32_t addr = next_value(&i);
    if (addr == NO_VALUE)
       return false;

    uint32_t count = next_value(&i);
    if (count == NO_VALUE)
        return false;

    for (size_t j = 0; j < count; ++j) {
        uint32_t data = next_value(&i);
        if (data == NO_VALUE)
            return false;

        if (!memory::set(addr + j, data)) {
            printf_P(PSTR("- %04X\n"), addr + j);
            return true;
        }
    }

    puts_P(PSTR("+"));
    
    return true;
}

static bool step_cycle()
{
    z80::StepCycleStatus ss = z80::step_cycle();
    unsigned int data = (ss.mem_pins.wr == 0 || ss.mem_pins.rd == 0) ? ss.data : 0xff;
    unsigned int addr = (ss.mem_pins.wr == 0 || ss.mem_pins.rd == 0) ? ss.addr : 0xffff;
    printf_P(PSTR("+ %x %x %d %d %d %d %d %d %d %d\n"),
             data, addr, ss.m1, ss.iorq, ss.busak, ss.wait, ss.int_,
             ss.mem_pins.wr, ss.mem_pins.rd, ss.mem_pins.mreq);
    return true;
}

static bool step()
{
    z80::LastOp c = z80::step();
    printf_P(PSTR("+ %x\n"), c.pc);
    return true;
}

static void print_step_status(z80::StepStatus ss)
{
    auto prhex = [](uint16_t v) { printf_P(PSTR("%x "), v); };

    putchar('+');
    putchar(' ');

    if (ss.has_info) {
        prhex(ss.af);
        prhex(ss.bc);
        prhex(ss.de);
        prhex(ss.hl);
        prhex(ss.afx);
        prhex(ss.bcx);
        prhex(ss.dex);
        prhex(ss.hlx);
        prhex(ss.ix);
        prhex(ss.iy);
        prhex(ss.sp);
        prhex(ss.pc);
        for (size_t i = 0; i < 8; ++i)
            prhex(ss.stack[i]);
        prhex(ss.bank);
        prhex(ss.ramonly);
    } else {
        prhex(ss.pc);
    }

    putchar('\n');
}

static bool step_nmi()
{
    z80::StepStatus ss = z80::step_nmi();
    print_step_status(ss);
    return true;
}

static bool next()
{
    z80::StepStatus ss = z80::next();
    print_step_status(ss);
    return true;
}

static bool reset()
{
    z80::reset();
    z80::step();
    puts_P(PSTR("+"));
    return true;
}

static bool swap_bkp(size_t i)
{
    uint32_t bkp = next_value(&i);
    if (bkp == NO_VALUE)
        return false;

    z80::bkp_swap((uint16_t) bkp);

    printf_P(PSTR("+ "));
    for (size_t j = 0; j < MAX_BKP; ++j)
        if (z80::bkp_list()[j] != NO_BKP)
            printf_P(PSTR("%x "), z80::bkp_list()[j]);
    printf_P(PSTR("\n"));

    return true;
}

static bool debug_run()
{
    z80::LastOp c = z80::debug_run();
    printf_P(PSTR("+ %x\n"), c.pc);
    return true;
}

static bool parse_input()
{
    size_t i = 0;
    while (buffer::input[i] == ' ') ++i;  // trim left

    if (buffer::input[i+1] != ' ' && buffer::input[i+1] != 0)
        return false;

    switch (buffer::input[i]) {
        case 'A':
            puts_P(PSTR("+"));
            return true;
        case 'P':
            return post_tests();
        case 'R':
            return read_memory(i + 2);
        case 'X':
            return reset();
        case 'W':
            return write_memory(i + 2);
        case 'S':
            return step();
        case 'N':
            return step_nmi();
        case 's':
            return step_cycle();
        case 'n':
            return next();
        case 'D':
            return debug_run();
        case 'B':
            return swap_bkp(i + 2);
    }

    return false;
}

void listen()
{
    bool ok = input_buffer();
    if (!ok) {
        puts_P(PSTR("x V"));
    } else {
        if (!parse_input())
            puts_P(PSTR("x ?"));
    }
}

}
