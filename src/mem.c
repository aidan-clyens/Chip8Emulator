#include "mem.h"

void mem_init() {
    memory = malloc(MEMORY_SIZE * sizeof(byte_t));
    graphics_display = &memory[DISPLAY_SPACE];
    memset(graphics_display, 0, DISPLAY_SIZE);
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