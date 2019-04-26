#include "debug.h"

/*
 * Print memory contents
 */
void print_mem() {
    for (int j = 0; j < mem_size; j++) {
        printf("%x ", memory[j]);
    }
    printf("\n");
}

void print_display() {
    for (int i=0; i < DISPLAY_SIZE; i++) {
        printf("%x ", display[i]);
    }
    printf("\n");
}

/*
 * Print register values
 */
void print_reg() {
    for (int j = 0; j < sizeof(v); j++) {
        printf("v%x      %x\n", j, v[j]);
    }
    printf("I       %x\n", I);
    printf("pc      %x\n", pc);
    printf("sp      %x\n", sp);
}