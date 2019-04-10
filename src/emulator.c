#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

// Set DEBUG to 1 to print disassembled opcodes
#define DEBUG   1

// Types
typedef unsigned char byte_t;
typedef unsigned short word_t;

// Registers
byte_t v[16];
word_t I;
word_t pc;
word_t sp;

word_t opcode;

// Memory
byte_t *memory;
int mem_size;

// Display
byte_t *display;

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
byte_t key[16];

// Timers
byte_t delay_timer;

// Debug
/*
 * Print memory contents
 */
void print_mem() {
    for (int j = 0; j < mem_size; j++) {
        printf("%x ", memory[j]);
    }
    printf("\n");
}

/*
 * Print register values
 */
void print_reg() {
    for (int j=0; j < sizeof(v); j++) {
        printf("v%x      %x\n", j, v[j]);
    }
    printf("I       %x\n", I);
    printf("pc      %x\n", pc);
    printf("sp      %x\n", sp);
}

/*
 * Initialize CPU registers
 */
int initialize() {
    memory = malloc(4096 * sizeof(byte_t));
    display = &memory[0x200];
    I = 0;
    pc = 0x0200;
    sp = 0x0EA0;
    opcode = 0;

    delay_timer = 0;

    return 1;
}

// Stack
/*
 * Push to stack
 */
int stack_push(word_t val) {
    // If val is a word
    if (val > 0xFF) {
        byte_t high = (val & 0xFF00) >> 8;
        byte_t low = val & 0xFF;
        
        sp -= 2;
        memory[sp] = high;
        sp -=2;
        memory[sp] = low;
    }
    // If val is a byte
    else {
        sp -= 2;
        memory[sp] = (byte_t) val;
    }
}

/*
 * Pop from stack
 */
byte_t stack_pop() {
    byte_t val = memory[sp];
    sp += 2;

    return val;
}

// File Loading
/*
 * Load the game file into memory
 */
int read_file(char *game_name) {
    char file_path[strlen("c8games/") + strlen(game_name) + 1];
    byte_t buffer[2];
    FILE *file;

    strcpy(file_path, "c8games/");
    strcat(file_path, game_name);

    file = fopen(file_path, "rb");

    word_t i = pc;
    while (fread(buffer, sizeof(buffer), 1, file) != 0) {    
        memory[i] = buffer[0];
        memory[i+1] = buffer[1];

        i += 2;
    }
    mem_size = i;

    #if DEBUG == 1
    print_mem();
    #endif

    fclose(file);

    return 1;
}

// CPU
/*
 * Complete a CPU cycle
 */
int complete_cycle() {
    // Get opcode from program memory
    opcode = (memory[pc] << 8) | memory[pc+1];

    // Decode opcode
    // No Operation
    if (opcode == 0x0000) {
        #if DEBUG == 1
        printf("%x      NOP\n", pc);
        #endif
    }
    // Erase Screen
    else if (opcode == 0x00E0) {
        #if DEBUG == 1
        printf("%x      ERASE\n", pc);
        #endif
        // Clear screen
    }
    // Return from Subroutine
    else if (opcode == 0x00EE) {
        #if DEBUG == 1
        printf("%x      RETURN\n", pc);
        #endif

        // Pop pc from stack
        byte_t low = stack_pop();
        byte_t high = stack_pop();

        pc = (high << 8) | low;
    }
    // Jump
    else if ((opcode & 0xF000) == 0x1000) {
        word_t val = opcode & 0xFFF;

        #if DEBUG == 1
        printf("%x      GOTO %x\n", pc, val);
        #endif

        pc = val-2;
    }
    // Call Subroutine
    else if ((opcode & 0xF000) == 0x2000) {
        word_t val = opcode & 0xFFF;

        #if DEBUG == 1
        printf("%x      DO %x\n", pc, val);
        #endif

        // Push pc to stack and set pc to val
        stack_push(pc);

        pc = val-2;
    }
    // Conditional Statements
    else if ((opcode & 0xF000) == 0x3000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #if DEBUG == 1
        printf("%x      SKF v%x=%x\n", pc, reg, val);
        #endif

        if (v[reg] == val) {
            pc += 2;
        }
    }
    else if ((opcode & 0xF000) == 0x4000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #if DEBUG == 1
        printf("%x      SKF v%x!=%x\n", pc, reg, val);
        #endif

        if (v[reg] != val) {
            pc += 2;
        }
    }
    // Move
    else if ((opcode & 0xF000) == 0x6000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #if DEBUG == 1
        printf("%x      MOV v%x, %x\n", pc, reg, val);
        #endif

        v[reg] = val;
    }
    // Addition
    else if ((opcode & 0xF000) == 0x7000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #if DEBUG == 1
        printf("%x      v%x=v%x+%x\n", pc, reg, reg, val);
        #endif

        v[reg] += val;
    }
    // Logical Operations
    else if ((opcode & 0xF000) == 0x8000) {
        byte_t regX = (opcode & 0xF00) >> 8;
        byte_t regY = (opcode & 0xF0) >> 4;
        
        // Copy VY to VX
        if ((opcode & 0xF00F) == 0x8000) {
            #if DEBUG == 1
            printf("%x      v%x=v%x\n", pc, regX, regY);
            #endif

            v[regX] = v[regY];
        }
        // Logical OR VX and VY
        else if ((opcode & 0xF00F) == 0x8001) {
            #if DEBUG == 1
            printf("%x      v%x=v%x|v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] | v[regY];
        }
        // Logical AND VX and VY
        else if ((opcode & 0xF00F) == 0x8002) {
            #if DEBUG == 1
            printf("%x      v%x=v%x&v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] & v[regY];
        }
    }
    // Set Memory Pointer
    else if ((opcode & 0xF000) == 0xA000) {
        word_t val = opcode & 0xFFF;

        #if DEBUG == 1
        printf("%x      MOV I, %x\n", pc, val);
        #endif

        I = val;
    }
    // Random Byte
    else if ((opcode & 0xF000) == 0xC000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;
        byte_t num = rand() % 256;

        #if DEBUG == 1
        printf("%x      v%x=RAND&%x\n", pc, reg, val);
        #endif

        num = num & val;
    }
    // Draw Sprite
    else if ((opcode & 0xF000) == 0xD000) {
        byte_t x = (opcode & 0xF00) >> 8;
        byte_t y = (opcode & 0xF0) >> 4;
        byte_t N = opcode & 0xF;

        #if DEBUG == 1
        printf("%x      SPRITE v%x, v%x, %x\n", pc, x, y, N);
        printf("v[%x] = %x, v[%x] = %x\n", x, v[x], y, v[y]);

        for (int j=0; j<N; j++) {
            printf("%x", memory[I+j]);
        }
        printf("\n");
        #endif
        
        // TODO: Use OpenGL to draw graphics
    }
    else if ((opcode & 0xF000) == 0xE000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        
        // Skip if Key Down
        if ((opcode & 0xF0FF) == 0xE09E) {
            // TODO: Skip if key is down

            #if DEBUG == 1
            printf("%x      SKF v%x=KEY\n", pc, reg);
            #endif
        }
        // Skip if Key is not Down
        else if ((opcode & 0xF0FF) == 0xE0A1) {
            // TODO: Skip if key is not down
            
            #if DEBUG == 1
            printf("%x      SKF v%x!=KEY\n", pc, reg);
            #endif
        }
    }
    else if ((opcode & 0xF000) == 0xF000) {
        if (opcode == 0xF000) {
            // TODO
            
            #if DEBUG == 1
            printf("%x      STOP\n", pc);
            #endif
        }
        // Get Timer Value
        else if ((opcode & 0xF0FF) == 0xF007) {
            byte_t reg = (opcode & 0xF00) >> 8;

            #if DEBUG == 1
            printf("%x      v%x=TIME\n", pc, reg);
            #endif
            
            v[reg] = delay_timer;
        }
        // Get Key Value
        else if ((opcode & 0xF0FF) == 0xF00A) {
            byte_t reg = (opcode & 0xF00) >> 8;

            // TODO: Get key value

            #if DEBUG == 1
            printf("%x      v%x=KEY\n", pc, reg);
            #endif
        }
        // Set Timer Value
        else if ((opcode & 0xF0FF) == 0xF015) {
            byte_t reg = (opcode & 0xF00) >> 8;
            
            #if DEBUG == 1
            printf("%x      TIME=v%x\n", pc, reg);
            #endif

            delay_timer = v[reg];
        }
    }
    else {
        #if DEBUG == 1
        printf("Unknown opcode: %x\n", opcode);
        #endif
    }
    
    // Decrement timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    // Increment Program Counter
    pc += 2;    

    return 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Enter a game name.\n");
        return -1;
    }
    
    char *game_name = argv[1];
    time_t t;

    initialize();
    read_file(game_name);

    srand((unsigned) time(&t));

    for (;;) {
        complete_cycle();
        usleep(1000*50);
    }

    free(memory);

    return 0;
}
