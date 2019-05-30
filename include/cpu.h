#ifndef CPU_H
#define CPU_H

#include "typedefs.h"
#include "mem.h"
#include "stack.h"
#include "graphics.h"
#include "utils.h"

// Registers
byte_t v[16];
word_t I;
word_t pc;
word_t sp;

// Timers
byte_t delay_timer;

double cpu_period_us;

void cpu_init();
int cpu_complete_cycle();
void cpu_run_instruction(word_t opcode);
word_t cpu_get_opcode();

#endif // CPU_H