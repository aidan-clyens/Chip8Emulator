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

#define DISPLAY_SPACE 0x100
#define PROGRAM_SPACE 0x200
#define STACK_SPACE 0x0EA0

#define CPU_CLOCK_HZ 150

// Types
typedef unsigned char byte_t;
typedef unsigned short word_t;

// Global Variables
extern byte_t v[16];
extern word_t I;
extern word_t pc;
extern word_t sp;

extern byte_t *graphics_display;

extern byte_t key_pressed;

#endif
