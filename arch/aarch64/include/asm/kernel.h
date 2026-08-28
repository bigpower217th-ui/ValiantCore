#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

typedef uint64_t addr_t;


/*  UART Serial Display port function definitions */
void uart_init();
void uart_putc(char c);
void uart_print(const char *s);
char uart_getc(void);
int  uart_has_input(void);
void kernel_panic_aarch64(uint64_t esr, uint64_t elr);
int ahci_init(void);
int ahci_read(uint64_t lba, uint16_t count, void *buf);
int ahci_write(uint64_t lba, uint16_t count, const void *buf);
int      fb64_init(uint64_t addr, uint32_t pitch,
                   uint32_t width, uint32_t height, uint8_t bpp);
int      fb64_is_ready(void);
uint32_t fb64_get_width(void);
uint32_t fb64_get_height(void);
void     fb64_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void     fb64_fill_rect(uint32_t x, uint32_t y, uint32_t w,
                        uint32_t h, uint32_t color);
void     fb64_clear(uint32_t color);
void     fb64_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void     fb64_draw_circle(int cx, int cy, int r, uint32_t color);
void     fb64_fill_circle(int cx, int cy, int r, uint32_t color);

#endif /* KERNEL_H */

