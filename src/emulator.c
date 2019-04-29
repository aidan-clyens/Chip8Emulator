#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "typedefs.h"
#include "stack.h"
#include "file.h"
#include "debug.h"
#include "graphics.h"

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
int draw_flag;

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

// Timers
byte_t delay_timer;

// Misc.
int cycle_count;

void game_loop();
void on_keypress(byte_t key, int x, int y);

/*
 * Initialize CPU registers
 */
int initialize(int argc, char **argv) {
    memory = malloc(4096 * sizeof(byte_t));
    display = &memory[DISPLAY_SPACE];
    memset(display, 0, DISPLAY_SIZE);

    for (int i=0; i < 80; i++) {
        memory[i] = fontset[i];
    }

    init_graphics(argc, argv, game_loop, on_keypress);

    draw_flag = 0;

    I = 0;
    pc = PROGRAM_SPACE;
    sp = STACK_SPACE;
    opcode = 0;

    delay_timer = 0;

    cycle_count = 0;

    key_pressed = -1;

    return 1;
}

// CPU
/*
 * Complete a CPU cycle
 */
int complete_cycle() {
    if (--cycle_count > 0) {
        return 0;
    }
    
    // Get opcode from program memory
    opcode = (memory[pc] << 8) | memory[pc+1];

    cycle_count = 1;

    // Decode opcode
    // No Operation
    if (opcode == 0x0000) {
        #ifdef DEBUG
        printf("%x      NOP\n", pc);
        #endif
    }
    // Erase Screen
    else if (opcode == 0x00E0) {
        #ifdef DEBUG
        printf("%x      ERASE\n", pc);
        #endif
        
        memset(display, 0, DISPLAY_SIZE);
        draw_flag = 1;
    }
    // Return from Subroutine
    else if (opcode == 0x00EE) {
        #ifdef DEBUG
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

        #ifdef DEBUG
        printf("%x      GOTO %x\n", pc, val);
        #endif

        pc = val-2;
    }
    // Call Subroutine
    else if ((opcode & 0xF000) == 0x2000) {
        word_t val = opcode & 0xFFF;

        #ifdef DEBUG
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

        #ifdef DEBUG
        printf("%x      SKF v%x=%x\n", pc, reg, val);
        printf("%x      v%x=%x\n", pc, reg, v[reg]);
        #endif

        if (v[reg] == val) {
            pc += 2;
        }
    }
    else if ((opcode & 0xF000) == 0x4000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #ifdef DEBUG
        printf("%x      SKF v%x!=%x\n", pc, reg, val);
        printf("%x      v%x=%x\n", pc, reg, v[reg]);
        #endif

        if (v[reg] != val) {
            pc += 2;
        }
    }
    // Move
    else if ((opcode & 0xF000) == 0x6000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #ifdef DEBUG
        printf("%x      MOV v%x, %x\n", pc, reg, val);
        #endif

        v[reg] = val;
    }
    // Addition
    else if ((opcode & 0xF000) == 0x7000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        #ifdef DEBUG
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
            #ifdef DEBUG
            printf("%x      v%x=v%x\n", pc, regX, regY);
            #endif

            v[regX] = v[regY];
        }
        // Logical OR VX and VY
        else if ((opcode & 0xF00F) == 0x8001) {
            #ifdef DEBUG
            printf("%x      v%x=v%x|v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] | v[regY];
        }
        // Logical AND VX and VY
        else if ((opcode & 0xF00F) == 0x8002) {
            #ifdef DEBUG
            printf("%x      v%x=v%x&v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] & v[regY];
        }
        else if ((opcode & 0xF00F) == 0x8003) {
            #ifdef DEBUG
            printf("%x      v%x=v%xXORv%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] ^ v[regY];
        }
        else if ((opcode & 0xF00F) == 0x8004) {
            #ifdef DEBUG
            printf("%x      v%x=v%x+v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] + v[regY];
        }
        else if ((opcode & 0xF00F) == 0x8005) {
            #ifdef DEBUG
            printf("%x      v%x=v%x-v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] - v[regY];
        }
        else {
            #ifdef DEBUG
            printf("Unknown opcode: %x\n", opcode);
            #endif
        }
    }
    // Skip Instruction
    else if ((opcode & 0xF00F) == 0x9000) {
        byte_t regX = (opcode & 0xF00) >> 8;
        byte_t regY = (opcode & 0xF0) >> 4;

        #ifdef DEBUG
        printf("%x      SKF v%x!=v%x\n", pc, regX, regY);
        #endif

        if (v[regX] != v[regY]) {
            pc += 2;
        }
    }
    // Set Memory Pointer
    else if ((opcode & 0xF000) == 0xA000) {
        word_t val = opcode & 0xFFF;

        #ifdef DEBUG
        printf("%x      MOV I, %x\n", pc, val);
        #endif

        I = val;
    }
    // Random Byte
    else if ((opcode & 0xF000) == 0xC000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;
        byte_t num = rand() % 256;

        #ifdef DEBUG
        printf("%x      v%x=RAND&%x\n", pc, reg, val);
        printf("%x      RAND=%x\n", pc, num);
        #endif

        v[reg] = val & num;
    }
    // Draw Sprite
    else if ((opcode & 0xF000) == 0xD000) {
        byte_t x = (opcode & 0xF00) >> 8;
        byte_t y = (opcode & 0xF0) >> 4;
        byte_t N = opcode & 0xF;

        #ifdef DEBUG
        printf("%x      SPRITE v%x, v%x, %x\n", pc, x, y, N);
        printf("v[%x] = %x, v[%x] = %x\n", x, v[x], y, v[y]);

        for (int j=0; j<N; j++) {
            printf("%x", memory[I+j]);
        }
        printf("\n");
        #endif
        
        cycle_count = 4;
        // TODO: Use SDL to draw graphics
        load_sprite(v[x], v[y], N);

        draw_flag = 1;
    }
    else if ((opcode & 0xF000) == 0xE000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        
        // Skip if Key Down
        if ((opcode & 0xF0FF) == 0xE09E) {
            // TODO: Skip if key is down

            #ifdef DEBUG
            printf("%x      SKF v%x=KEY\n", pc, reg);
            printf("         v%x=%x, KEY=%x\n", reg, v[reg], key_pressed);
            #endif

            if (v[reg] == key_pressed) {
                pc += 2;
            }
        }
        // Skip if Key is not Down
        else if ((opcode & 0xF0FF) == 0xE0A1) {
            // TODO: Skip if key is not down
            
            #ifdef DEBUG
            printf("%x      SKF v%x!=KEY\n", pc, reg);
            printf("         v%x=%x, KEY=%x\n", reg, v[reg], key_pressed);
            #endif

            if (v[reg] != key_pressed) {
                pc += 2;
            }
        }
    }
    else if ((opcode & 0xF000) == 0xF000) {
        if (opcode == 0xF000) {
            // TODO
            
            #ifdef DEBUG
            printf("%x      STOP\n", pc);
            #endif
        }
        // Get Timer Value
        else if ((opcode & 0xF0FF) == 0xF007) {
            byte_t reg = (opcode & 0xF00) >> 8;

            #ifdef DEBUG
            printf("%x      v%x=TIME\n", pc, reg);
            #endif
            
            v[reg] = delay_timer;
        }
        // Get Key Value
        else if ((opcode & 0xF0FF) == 0xF00A) {
            byte_t reg = (opcode & 0xF00) >> 8;

            // TODO: Get key value

            #ifdef DEBUG
            printf("%x      v%x=KEY\n", pc, reg);
            #endif
        }
        // Set Timer Value
        else if ((opcode & 0xF0FF) == 0xF015) {
            byte_t reg = (opcode & 0xF00) >> 8;
            
            #ifdef DEBUG
            printf("%x      TIME=v%x\n", pc, reg);
            #endif

            delay_timer = v[reg];
        }
    }
    else {
        #ifdef DEBUG
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

void game_loop() {
    complete_cycle();

    if (draw_flag) {
        draw_flag = 0;

        draw();
    }

    usleep(1000 * REFRESH_RATE_MS);
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
            key_pressed = -1;
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

    update_screen();

    free(memory);

    return 0;
}
