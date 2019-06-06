#include "chip8.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Enter a game name.\n");
        return -1;
    }
    
    char *game_name = argv[1];
    time_t t;

    chip8_initialize(game_name);

    srand((unsigned) time(&t));

    graphics_update_screen();

    chip8_close();

    return 0;
}
