#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "typedefs.h"
#include "file.h"
#include "graphics.h"
#include "mem.h"
#include "cpu.h"
#include "utils.h"

// Keys
uint8_t key_pressed;

void chip8_initialize(int argc, char **argv);
void chip8_close();
void chip8_game_loop();
void chip8_on_keypress(uint8_t key, int x, int y);

#endif // CHIP8_H