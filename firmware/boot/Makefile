# Project name
PROJECT=fortuna4-boot

# Object files (one for each source file)
OBJECTS=main.o

# Avrdude configuration to connect on the PI. See avrdude_gpio.conf.
# Can be one of: pi_rst, pi_cs0, pi_cs1 or pi_gpio, depending on the RESET line in AVR.
AVRDUDE_CONFIG=pi_rst

# AVR fuses: internal oscillator (see https://www.engbedded.com/fusecalc/)
FUSES=-U lfuse:w:0xe4:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

# Microcontroller
MCU_GCC=attiny2313
MCU_AVRDUDE=attiny2313

# Microcontroller speed, in Hz. Used by avr-libc for delays.
F_CPU=8000000UL

# Serial speed, in baud. Used by avr-libc for UART speed calculation.
BAUD=9600

#
# Rules
#

AVRDUDE_FLAGS=-p ${MCU_AVRDUDE} -C +./avrdude_gpio.conf -c ${AVRDUDE_CONFIG} -V
CC=avr-gcc
WARNINGS=-Wall -Wextra \
	 -Wformat=2 -Wno-unused-parameter -Wshadow \
	 -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	 -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wjump-misses-init -Wlogical-op
CPPFLAGS=-std=c11 ${WARNINGS} -Os -DF_CPU=${F_CPU} -DBAUD=${BAUD} -mmcu=${MCU_GCC} -ffunction-sections -fdata-sections -mcall-prologues

all: ${PROJECT}.hex

${PROJECT}.hex: ${PROJECT}.elf
	avr-objcopy -j .text -j .data -O ihex $< $@
	arm-linux-gnueabihf-size ${PROJECT}.hex

${PROJECT}.elf: ${OBJECTS}
	$(CC) -mmcu=${MCU_GCC} -o $@ $^ -Wl,--gc-sections
	avr-size -C --mcu=${MCU_GCC} ${PROJECT}.elf

test-connection:
	sudo avrdude ${AVRDUDE_FLAGS}

upload: ${PROJECT}.hex
	sudo avrdude ${AVRDUDE_FLAGS} -U flash:w:$<:i

fuse:
	sudo avrdude ${AVRDUDE_FLAGS} ${FUSES}

size: ${PROJECT}.elf
	avr-size -C --mcu=${MCU_GCC} ${PROJECT}.elf

talk:
	@echo "Press Ctrl+A and then Shift+K to exit."
	@sleep 1
	screen /dev/ttyAMA0 ${BAUD}

clean:
	rm -f ${OBJECTS} ${PROJECT}.elf ${PROJECT}.hex

# vim: set ts=8 sts=8 sw=8 noexpandtab:
