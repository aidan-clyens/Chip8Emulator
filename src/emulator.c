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

double cpu_period_us;

// Display
byte_t *display;
int graphics_draw_flag;

// Font Set
byte_t fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Keys
byte_t key_pressed;

void game_loop();
void on_keypress(byte_t key, int x, int y);

/*
 * Initialize CPU registers
 */
int initialize(int argc, char **argv) {
    mem_init();

    // Load fontset
    for (int i=0; i < 80; i++) {
        mem_write(i, fontset[i]);
    }

    graphics_init(argc, argv, game_loop, on_keypress);
    
    cpu_init();

    graphics_draw_flag = 0;

    key_pressed = -1;

    cpu_period_us = 1000.0 / CPU_CLOCK_HZ;

    return 1;
}

void game_loop() {
    int cpu_cycle_elapsed_time = cpu_complete_cycle();
    double delay_time = 0;
    
    if (cpu_cycle_elapsed_time < cpu_period_us) {
        delay_time = cpu_period_us - cpu_cycle_elapsed_time;
    }

    if (graphics_draw_flag) {
        graphics_draw_flag = 0;

        graphics_draw();
    }

    usleep(delay_time);
}

void on_keypress(byte_t key, int x, int y) {
    switch (key) {
        case '1':
            key_pressed = 0x1;
            break;
        case '2':
            key_pressed = 0x2;
            break;
        case '3':
            key_pressed = 0x3;
            break;
        case '4':
            key_pressed = 0xC;
            break;
        case 'q':
            key_pressed = 0x4;
            break;
        case 'w':
            key_pressed = 0x5;
            break;
        case 'e':
            key_pressed = 0x6;
            break;
        case 'r':
            key_pressed = 0xD;
            break;
        case 'a':
            key_pressed = 0x7;
            break;
        case 's':
            key_pressed = 0x8;
            break;
        case 'd':
            key_pressed = 0x9;
            break;
        case 'f':
            key_pressed = 0xE;
            break;
        case 'z':
            key_pressed = 0xA;
            break;
        case 'x':
            key_pressed = 0x0;
            break;
        case 'c':
            key_pressed = 0xB;
            break;
        case 'v':
            key_pressed = 0xF;
            break;
        default:
            key_pressed = 0xFF;
            break;
    }

    #ifdef DEBUG
    printf("key_pressed=%x\n", key_pressed);
    #endif
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Enter a game name.\n");
        return -1;
    }
    
    char *game_name = argv[1];
    time_t t;

    initialize(argc, argv);
    read_file(game_name);

    srand((unsigned) time(&t));

    graphics_update_screen();

    mem_free();

    return 0;
}
