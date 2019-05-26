#ifndef CPU_H
#define CPU_H

#include "typedefs.h"
#include "mem.h"
#include "stack.h"
#include "graphics.h"

// Registers
byte_t v[16];
word_t I;
word_t pc;
word_t sp;

// Timers
byte_t delay_timer;

void init_cpu();
void complete_cycle();
void run_instruction(word_t opcode);
word_t get_opcode();

#endif // CPU_H