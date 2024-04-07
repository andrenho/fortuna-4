#include <stdlib.h>
#include <stdio.h>
#include "comm.h"
#include "fortuna.h"

#include "z80/Z80.h"

static uint16_t i(uint16_t v)
{
    return (v << 8) | (v >> 8);
}

int main()
{
    char serial_port[100];
    if (!comm_init(serial_port))
        exit(EXIT_FAILURE);

    printf("Listening on serial port: %s\n", serial_port);

    fortuna_init();

    // read serial port
    while (1) {
        char buffer[1024];
        comm_readline(buffer, sizeof buffer);
        comm_echo(buffer);

        char cmd;
        int values[512];
        size_t n_values = comm_scanf(buffer, &cmd, values, sizeof values);

        switch (cmd) {
            case 'A':  // ack
                comm_printf("+\n");
                break;
            case 'X':  // reest
                fortuna_reset();
                comm_printf("+\n");
                break;
            case 'R':  // read memory
                comm_printf("+ %x ", values[1]);
                for (int i = values[0]; i < (values[0] + values[1]); ++i)
                    comm_printf("%x ", RdZ80(i));
                comm_printf("\n");
                break;
            case 'W':  // write memory
                fortuna_write_to_rom(values[0], &values[2], n_values - 2);
                comm_printf("+\n");
                break;
            case 'P':  // post
                comm_printf("+ +a +b +c +d +e +f +g +h\n");
                break;
            case 's':  // step-cycle
                comm_printf("+ 0 0 0 0 0 0 0 0 0 0\n");
                break;
            case 'S': { // step
                    Z80 const* z80 = fortuna_step();
                    comm_printf("+ %x\n", z80->PC);
                }
                break;
            case 'N': {  // step nmi
                    // af, bc, de, hl, afx, bcx, dex, hlx, ix, iy, sp, pc, st0, st1, st2, st3, st4, st5, st6, st7
                    Z80 const* z80 = fortuna_step();
                    comm_printf("+ %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
                                i(z80->AF.W), i(z80->BC.W), i(z80->DE.W), i(z80->HL.W),
                                i(z80->AF1.W), i( z80->BC1.W), i(z80->DE1.W), i(z80->HL1.W),
                                i(z80->IX.W), i(z80->IY.W), i(z80->SP.W), i(z80->PC.W),
                                0, 0, 0, 0, 0, 0, 0, 0);
                }
                break;
            default:
                comm_printf("x\n");
        }
    }

    return 0;
}
