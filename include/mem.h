#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <string.h>
#include "typedefs.h"

void mem_init();
void mem_free();
byte_t mem_read(int address);
void mem_write(int address, byte_t data);

byte_t *memory;
int mem_size;

#endif