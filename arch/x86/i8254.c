/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "../../include/kernel.h"
#include "../../include/scheduler.h"
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42
#define PIT_CMD      0x43

#define PIT_MODE_SQUARE 0x36

#define PIT_BASE_FREQ   1193182

static uint32_t pit_ticks = 0;
static uint32_t pit_hz    = 0;

void pit_init(uint32_t hz) {
    pit_hz = hz;

    uint32_t divisor = PIT_BASE_FREQ / hz;

    outb(PIT_CMD, PIT_MODE_SQUARE);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    kprint("[PIC] İnitialized.\n");
}

void pit_handler(void) {
   pit_ticks++;
   schedule_next();
}

uint32_t pit_get_ticks(void) { return pit_ticks; }
uint32_t pit_get_hz(void)    { return pit_hz;    }

void pit_sleep(uint32_t ms) {
    uint32_t ticks = (pit_hz * ms) / 1000;
    uint32_t start = pit_ticks;
    while ((pit_ticks - start) < ticks);
}
