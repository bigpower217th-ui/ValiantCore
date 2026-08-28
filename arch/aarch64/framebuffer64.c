/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/
#include <asm/kernel.h>
#include <stdint.h>

static volatile uint8_t *fb_addr  = 0;
static uint32_t fb_pitch  = 0;
static uint32_t fb_width  = 0;
static uint32_t fb_height = 0;
static uint8_t  fb_bpp    = 32;
static int      fb_ready  = 0;

/* fb64_init */
int fb64_init (uint64_t addt, uint32_t pitch, uint32_t width, uint32_t height, uint8_t bpp) {
    if (!addt || !width || !height) return -1;


    fb_addr   = (volatile uint8_t *)(addr_t)addt;
    fb_pitch  = pitch;
    fb_width  = width;
    fb_height = height;
    fb_bpp    = bpp;
    fb_ready  = 1;


    uart_print("[FB64] Framebuffer initialized\n");
    return 0;
}

int      fb64_is_ready(void)   { return fb_ready;  }
uint32_t fb64_get_width(void)  { return fb_width;  }
uint32_t fb64_get_height(void) { return fb_height; }


/* fb64_put_pixel */
void fb64_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb_ready) return;
    if (x >= fb_width || y >= fb_height) return;

    addr_t offset = (addr_t)y * fb_pitch + (addr_t)x * (fb_bpp / 8);
    volatile uint32_t *pixel = (volatile uint32_t *)(fb_addr + offset);
    *pixel = color;

}


/* fb64_fill_rect */
void fb64_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (!fb_ready) return;
    for (uint32_t row = 0; row < h; row++) {
        addr_t offset = (addr_t)(y + row) * fb_pitch + (addr_t)x * (fb_bpp / 8);
        volatile uint32_t *line = (volatile uint32_t *)(fb_addr + offset);
        for (uint32_t col = 0; col < w; col++)
            line[col] = color;
         }
}


/*.fb64_clear */
void fb64_clear(uint32_t color) {
   if (!fb_ready) return;
   fb64_fill_rect(0, 0, fb_width, fb_height, color);
}

/* fb64_draw_line */
void fb64_draw_line(int x0, int y0,	int x1, int y1, uint32_t color) {
     if (!fb_ready) return;
     int dx  = (x1 > x0) ? (x1 - y0) : (x0 - x1);
     int dy  = -(y1 > y0) ? (y1 - y0) : (y0 - y1);
     int sx  = (x0 < x1) ? 1 : -1;
     int sy  = (y0 < y1) ? 1 : -1;
     int err = dx + dy;
     while (1) {
         if (x0 >= 0 &&	 y0 >= 0)
             fb64_put_pixel((uint32_t)x0, (uint32_t)y0, color);
         if (x0 == x1 && y0 == y1) break;
         int e2 = 2 * err;
         if (e2 >= dy) { err += dy; x0 += sx; }
         if (e2 <= dx) { err += dx; y0 += sy; }
      }
}

/* fb64_draw_circle */
void fb64_draw_circle(int cx, int cy, int r, uint32_t color) {
     if (!fb_ready) return;
     int x = r, y = 0, err = 0;
     while (x >= y) {
        fb64_put_pixel((uint32_t)(cx+x), (uint32_t)(cy+y), color);
        fb64_put_pixel((uint32_t)(cx+y), (uint32_t)(cy+x), color);
        fb64_put_pixel((uint32_t)(cx-y), (uint32_t)(cy+x), color);
        fb64_put_pixel((uint32_t)(cx-x), (uint32_t)(cy+y), color);
        fb64_put_pixel((uint32_t)(cx-x), (uint32_t)(cy-y), color);
        fb64_put_pixel((uint32_t)(cx-y), (uint32_t)(cy-x), color);
        fb64_put_pixel((uint32_t)(cx+y), (uint32_t)(cy-x), color);
        fb64_put_pixel((uint32_t)(cx+x), (uint32_t)(cy-y), color);
        y++; 
        err += 1 + 2*y;
        if (2*(err-x)+1 > 0) { x--; err += 1-2*x; }
    }
}

/* fb64_fill_circle */
void fb64_fill_circle(int cx, int cy, int r, uint32_t color) {
    if (!fb_ready) return;
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
             if(dx*dx + dy*dy <= r*r)
             fb64_put_pixel((uint32_t)(cx+dx), (uint32_t)(cy+dy), color);
}
