#include "stack.h"

/*
 * Push to stack
 */
int stack_push(uint16_t val) {
    // If val is a word
    if (val > 0xFF) {
        uint8_t high = (val & 0xFF00) >> 8;
        uint8_t low = val & 0xFF;

        sp -= 2;
        mem_write(sp, high);
        sp -= 2;
        mem_write(sp, low);
    }
    // If val is a byte
    else {
        sp -= 2;
        mem_write(sp, (uint8_t)val);
    }
}

/*
 * Pop from stack
 */
uint8_t stack_pop() {
    uint8_t val = mem_read(sp);
    sp += 2;

    return val;
}