#include "comm.h"

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#if __APPLE__
#  include <util.h>
#else
#  include <pty.h>
#  include <stdio.h>
#  include <termios.h>
#endif

#define DEBUG 1

static int master_fd;

bool comm_init(char* serial_port)
{
    int slave_fd;

    if (openpty(&master_fd, &slave_fd, serial_port, NULL, NULL) == -1) {
        perror("openpty");
        return false;
    }

    // configure terminal settings
    struct termios tio;
    tcgetattr(master_fd, &tio);
    cfmakeraw(&tio);
    tcsetattr(master_fd, TCSANOW, &tio);

    return true;
}

void comm_readline(char* line, size_t max_size)
{
    size_t i = 0;
    while (i < max_size) {
        size_t num_bytes = read(master_fd, &line[i], 1);   // TODO - poll
        if (num_bytes > 0) {
            if (line[i] == 13 || line[i] == 10) {
                line[i] = '\0';
#ifdef DEBUG
        printf("\e[1;32m%s\e[0m", line);
#endif
                return;
            }
            ++i;
        } else {
            usleep(10000);
        }
    }
}

void comm_echo(const char* str)
{
    write(master_fd, str, strlen(str));
    write(master_fd, &"\n", 1);
}

void comm_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buffer[1024];
    int sz = vsnprintf(buffer, sizeof buffer, fmt, args);
#ifdef DEBUG
        printf("\e[0;31m%s\e[0m", buffer);
#endif
    write(master_fd, buffer, sz);

    va_end(args);
}

size_t comm_scanf(const char* buf, char* cmd, int* values, size_t max_values)
{
    size_t n_values = 0;

    *cmd = buf[0];
    if (buf[1] == ' ') {
        size_t pos = 2;
        int nxt;
        int value;
        while (1) {
            if (buf[pos] == '\0')
                return n_values;
            if (sscanf(&buf[pos], "%x%n", &value, &nxt) == 0 || n_values >= max_values)
                return n_values;
            pos += nxt;
            values[n_values++] = value;
        }
    }

    return 0;
}
