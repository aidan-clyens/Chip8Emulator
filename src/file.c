#include "file.h"

/*
 * Load the game file into memory
 */
int read_file(char *game_name) {
    char file_path[strlen("c8games/") + strlen(game_name) + 1];
    uint8_t buffer[2];
    FILE *file;

    strcpy(file_path, "c8games/");
    strcat(file_path, game_name);

    file = fopen(file_path, "rb");

    uint16_t i = pc;
    while (fread(buffer, sizeof(buffer), 1, file) != 0) {
        mem_write(i, buffer[0]);
        mem_write(i + 1, buffer[1]);

        i += 2;
    }
    mem_size = i;

#ifdef DEBUG
    print_mem();
#endif

    fclose(file);

    return 1;
}