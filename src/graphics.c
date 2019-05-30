#include "graphics.h"

void keyboard_up(byte_t key, int x, int y) {
    key_pressed = -1;
}

void graphics_init(int argc, char **argv, void *loop, void *keypress) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("CHIP 8 EMULATOR");

    glutDisplayFunc(graphics_draw);
    glutIdleFunc(loop);
    glutKeyboardFunc(keypress);
    glutKeyboardUpFunc(keyboard_up);
    glutSetKeyRepeat(0);

    glClear(GL_COLOR_BUFFER_BIT);
    memset(screen, BLACK, SCREEN_WIDTH * SCREEN_HEIGHT * 3);
}

void graphics_load_sprite(int x, int y, int N) {
    int byte_index = x / 8;
    int bit_shift = x % 8;

    for (int yline = 0; yline < N; yline++) {
        byte_t pixel = mem_read(I + yline);
        byte_t next_mask = 0;
        int row_offset = 8 * y + 8 * yline;

        for (int xline = 0; xline < bit_shift; xline++) {
            next_mask = (next_mask >> 1) | (pixel & 0x1) << 7;
            pixel = pixel >> 1;
        }

        graphics_display[row_offset + byte_index] ^= pixel;
        graphics_display[row_offset + byte_index + 1] ^= next_mask;
    }
}

void graphics_load_pixel(int row, int col, byte_t color) {
    if (screen[row][col][0] == WHITE) {
        v[0xF] = 1;
    }

    row = SCREEN_HEIGHT - 1 - row;
    screen[row][col][0] = screen[row][col][1] = screen[row][col][2] = color;
}

void graphic_graphics_draw_cell(int row, int col, byte_t color) {
    int pixel_row = row * PIXEL_SIZE;
    int pixel_col = col * PIXEL_SIZE;

    graphics_load_pixel(pixel_row, pixel_col, color);

    for (int i = 0; i < PIXEL_SIZE; i++) {
        for (int j = 0; j < PIXEL_SIZE; j++) {
            graphics_load_pixel(pixel_row + i, pixel_col + j, color);
        }
    }
}

void graphics_draw() {
    int row = 0;
    int col = 0;
    int index = 0;

    glClear(GL_COLOR_BUFFER_BIT);

    memset(screen, BLACK, SCREEN_HEIGHT * SCREEN_WIDTH * 3);
    for (int i = 0; i < DISPLAY_SIZE; i++) {
        for (int j = 7; j >= 0; j--) {
            byte_t pixel = (graphics_display[i] >> j) & 0x1;
            byte_t color = (pixel == 1) ? WHITE : BLACK;

            graphic_graphics_draw_cell(row, col, color);

            index++;

            if (index > 0 && (index % DISPLAY_WIDTH == 0)) {
                col = 0;
                row++;
            }
            else {
                col++;
            }
        }
    }

    glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (void *)screen);
    glutSwapBuffers();
}

void graphics_update_screen() {
    glutMainLoop();
}
