PROJECT=fortunabox-emulator

OBJECTS=main.o comm.o fortuna.o z80/Z80.o

CFLAGS=-std=c11 -Wall -Wextra -O0 -ggdb

all: ${PROJECT}

${PROJECT}: ${OBJECTS}
	gcc -o $@ $^ ${CFLAGS}

clean:
	rm -f ${OBJECTS} ${PROJECT}