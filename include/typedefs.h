// Definitions
// Uncomment line below to print disassembled opcodes
#define DEBUG

#define DISPLAY_SIZE 256
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

#define DISPLAY_SPACE 0x100
#define PROGRAM_SPACE 0x200
#define STACK_SPACE 0x0EA0

// Types
typedef unsigned char byte_t;
typedef unsigned short word_t;

// Global Variables
extern byte_t v[16];
extern word_t I;
extern word_t pc;
extern word_t sp;

extern byte_t *memory;
extern int mem_size;