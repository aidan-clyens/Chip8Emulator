#include "stack.h"

/*
 * Push to stack
 */
int stack_push(word_t val) {
    // If val is a word
    if (val > 0xFF) {
        byte_t high = (val & 0xFF00) >> 8;
        byte_t low = val & 0xFF;

        sp -= 2;
        mem_write(sp, high);
        sp -= 2;
        mem_write(sp, low);
    }
    // If val is a byte
    else {
        sp -= 2;
        mem_write(sp, (byte_t)val);
    }
}

/*
 * Pop from stack
 */
byte_t stack_pop() {
    byte_t val = mem_read(sp);
    sp += 2;

    return val;
}