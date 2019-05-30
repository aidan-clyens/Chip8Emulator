#ifndef CPU_H
#define CPU_H

#include "typedefs.h"
#include "mem.h"
#include "stack.h"
#include "graphics.h"
#include "utils.h"

// Registers
uint8_t v[16];
uint16_t I;
uint16_t pc;
uint16_t sp;

// Timers
uint8_t delay_timer;

double cpu_period_us;

void cpu_init();
int cpu_complete_cycle();
void cpu_run_instruction(uint16_t opcode);
uint16_t cpu_get_opcode();

#endif // CPU_H