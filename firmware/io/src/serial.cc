#include "serial.hh"

#include "pico/stdlib.h"

namespace serial {

static volatile char last_char = 0x0;

static void input_cb([[maybe_unused]] void* data)
{
    last_char = getchar_timeout_us(0);
}

void init()
{
    stdio_usb_init();
    stdio_set_chars_available_callback(input_cb, nullptr);
}

uint8_t read()
{
    char data = last_char;
    last_char = 0x0;
    return 'a';
}

void write(uint8_t data)
{
    putchar_raw(data);
}

}