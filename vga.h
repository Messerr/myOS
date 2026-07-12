#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_WIDTH  320
#define VGA_HEIGHT 200
#define VGA_COLORS 256

/* Core functions */
void vga_init(void);
void vga_exit(void);  /* Return to text mode */

/* Drawing primitives */
void vga_clear(uint8_t color);
void vga_putpixel(int x, int y, uint8_t color);
uint8_t vga_getpixel(int x, int y);
void vga_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_rect(int x, int y, int w, int h, uint8_t color);
void vga_fillrect(int x, int y, int w, int h, uint8_t color);
void vga_circle(int cx, int cy, int r, uint8_t color);
void vga_fillcircle(int cx, int cy, int r, uint8_t color);

/* Palette */
void vga_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/* Double buffering */
void vga_swap(void);  /* Copy back buffer to screen */

/* Simple text in graphics mode */
void vga_putchar_at(int x, int y, char c, uint8_t color);
void vga_print(int x, int y, const char* str, uint8_t color);

#endif
