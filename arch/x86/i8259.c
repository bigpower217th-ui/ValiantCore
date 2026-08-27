/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/

#include "../../include/kernel.h"
#include <stdint.h>

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1
#define PIC_EOI    0x20

#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define PIC_READ_IRR 0x0A
#define PIC_READ_IRR 0x0A
#define PIC_READ_ISR 0x0B

void pic_init(void) {
   uint8_t mask1 = inb(PIC1_DATA);
   uint8_t mask2 = inb(PIC2_DATA);

   outb(PIC1_CMD,   ICW1_INIT | ICW1_ICW4);
   outb(0x80, 0);
   outb(PIC2_CMD,   ICW1_INIT | ICW1_ICW4);
   outb(0x80, 0);

   outb(PIC1_DATA,  PIC1_OFFSET);
   outb(0x80, 0);
   outb(PIC2_DATA,  PIC2_OFFSET);

   outb(PIC1_DATA, 0x04);
   outb(0x80, 0);
   outb(PIC2_DATA, 0x02);
   outb(0x80, 0);

   outb(PIC1_DATA, ICW4_8086);
   outb(0x80, 0);
   outb(PIC2_DATA, ICW4_8086);
   outb(0x80, 0);

   outb(PIC1_DATA, mask1);
   outb(PIC2_DATA, mask2);

   kprint("PIC İnitialized.\n");
}

void pic_send_eoi(uint8_t irq) {
   if (irq >= 8)
      outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);

}


void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
       port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -=8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t  value;

    if (irq < 8) {
       port = PIC1_DATA;
    } else {
       port = PIC2_DATA;
       irq  -= 8;
    }
   value = inb(port) & ~(1 << irq);
   outb(port, value);
}

static uint16_t pic_get_reg(uint8_t ocw3) {
   outb(PIC1_CMD, ocw3);
   outb(PIC2_CMD, ocw3);
    return ((uint16_t)inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

uint16_t pic_get_irr(void) { return pic_get_reg	(PIC_READ_IRR); }
uint16_t pic_get_isr(void) { return pic_get_reg(PIC_READ_ISR); }

void pic_disable(void) {
   outb(PIC2_DATA, 0xFF);
   outb(PIC1_DATA, 0xFF);
   kprint("[PIC] Disabled.\n");
}
