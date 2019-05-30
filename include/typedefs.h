// Definitions
// Uncomment line below to print disassembled opcodes
// #define DEBUG

#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#define DISPLAY_SIZE 256
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define PIXEL_SIZE 16
#define SCREEN_WIDTH DISPLAY_WIDTH*PIXEL_SIZE
#define SCREEN_HEIGHT DISPLAY_HEIGHT*PIXEL_SIZE

#define WHITE 0xFF
#define BLACK 0

#define MEMORY_SIZE 4096

#define DISPLAY_SPACE 0x100
#define PROGRAM_SPACE 0x200
#define STACK_SPACE 0x0EA0

#define CPU_CLOCK_HZ 150

// Types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

// Global Variables
extern uint8_t v[16];
extern uint16_t I;
extern uint16_t pc;
extern uint16_t sp;

extern uint8_t *graphics_display;

extern uint8_t key_pressed;

#endif
