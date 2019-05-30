#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <string.h>
#include "typedefs.h"

void mem_init();
void mem_free();
uint8_t mem_read(int address);
void mem_write(int address, uint8_t data);

uint8_t *memory;
int mem_size;

#endif