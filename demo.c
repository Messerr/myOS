#include "demo.h"
#include "vga.h"

void graphics_demo(void) {
    vga_init();

    /* Black background */
    vga_clear(0);

    /* Draw a colorful scene */

    /* Sky gradient */
    for (int y = 0; y < 100; y++) {
        uint8_t color = 1 + (y / 10);  /* dark blue gradient */
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_putpixel(x, y, color);
        }
    }

    /* Ground */
    vga_fillrect(0, 140, VGA_WIDTH, 60, 2);  /* dark green */

    /* Sun */
    vga_fillcircle(260, 40, 20, 14);  /* yellow */

    /* House body */
    vga_fillrect(100, 100, 80, 60, 4);  /* dark red */

    /* Roof */
    for (int i = 0; i < 30; i++) {
        vga_line(100 - i, 100 - i, 180 + i, 100 - i, 6); /* brown */
    }
    /* Proper triangle roof */
    for (int y = 0; y < 30; y++) {
        int half = y;
        vga_line(140 - half, 100 - 30 + y, 140 + half, 100 - 30 + y, 6);
    }

    /* Door */
    vga_fillrect(130, 120, 20, 40, 6);  /* brown */

    /* Windows */
    vga_fillrect(108, 110, 15, 15, 11);  /* cyan */
    vga_fillrect(158, 110, 15, 15, 11);  /* cyan */
    /* Window cross */
    vga_line(115, 110, 115, 125, 0);
    vga_line(108, 117, 123, 117, 0);
    vga_line(165, 110, 165, 125, 0);
    vga_line(158, 117, 173, 117, 0);

    /* Tree trunk */
    vga_fillrect(40, 110, 8, 40, 6);  /* brown */
    /* Tree leaves */
    vga_fillcircle(44, 100, 20, 10);  /* green */

    /* Clouds */
    vga_fillcircle(60, 25, 12, 15);
    vga_fillcircle(75, 22, 10, 15);
    vga_fillcircle(50, 22, 8, 15);

    vga_fillcircle(200, 35, 10, 15);
    vga_fillcircle(215, 32, 12, 15);

    /* Text */
    vga_print(10, 185, "MyOS Graphics Mode - 320x200 256 colors", 15);

    /* Flush to screen */
    vga_swap();
}
