#include "mem.h"

void mem_init() {
    memory = malloc(4096 * sizeof(byte_t));
    display = &memory[DISPLAY_SPACE];
    memset(display, 0, DISPLAY_SIZE);
}

void mem_free() {
    free(memory);
}

byte_t mem_read(int address) {
    return memory[address];
}

void mem_write(int address, byte_t data) {
    memory[address] = data;
}