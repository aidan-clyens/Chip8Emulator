#include "mem.h"

void mem_init() {
    memory = malloc(MEMORY_SIZE * sizeof(uint8_t));
    graphics_display = &memory[DISPLAY_SPACE];
    memset(graphics_display, 0, DISPLAY_SIZE);
}

void mem_free() {
    free(memory);
}

uint8_t mem_read(int address) {
    return memory[address];
}

void mem_write(int address, uint8_t data) {
    memory[address] = data;
}