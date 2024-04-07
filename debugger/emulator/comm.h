#ifndef EMULATOR_COMM_H
#define EMULATOR_COMM_H

#include <stdbool.h>
#include <stddef.h>

bool   comm_init(char* serial_port);
void   comm_readline(char* line, size_t max_size);
void   comm_echo(const char* str);
void   comm_printf(const char* fmt, ...);
size_t comm_scanf(const char* buf, char* cmd, int* values, size_t max_values);

#endif //EMULATOR_COMM_H
