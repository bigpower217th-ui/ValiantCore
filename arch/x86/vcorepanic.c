/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "../../include/kernel.h"
#include "../../include/paging.h"
#include "../../include/framebuffer.h"
#include <stdint.h>

#define COLOR_RED_BG       0xC0392B
#define COLOR_YELLOW       0xF1C40F
#define COLOR_YELLOW_TXT   0x000000
#define COLOR_WHITE        0xFFFFFF
#define COLOR_LIGHT        0xFFCDD2
#define COLOR_BLUE_BG      0x1565C0
#define COLOR_BLUE_BTN     0x1976D2
#define COLOR_BLUE_LIGHT   0xBBDEFB
#define COLOR_CYAN         0x00E5FF
#define COLOR_GREEN        0x69F0AE

void fb_clear(uint32_t color);
void font_draw_string_centered(uint32_t y, const char *s, uint32_t fg, uint32_t bg, int scale);

static void uint64_to_hex(uint64_t val, char *out) {
   const char hex[] = "0123456789ABCDEF";
   out[0] = '0'; out[1] = 'x';
   for (int i = 0; i < 16; i++) 
       out[2 + i] = hex[(val >> (60 - i * 4)) & 0xF];
   out[18] = '\0';
}

static void uint32_to_hex(uint32_t val, char *out) {
   const char hex[] = "0123456789ABCDEF";
   out[0] = '0'; out[1] = 'x';
   for (int i = 0; i < 8; i++)
       out[2 + i] = hex[(val >> (28 - i * 4)) & 0xF];
   out[10] = '\0';
}

static const char *panic_detect(uint32_t int_no) {
    switch (int_no) {
        case 0:   return "ERR_DIVIDE_BY_ZERO";
        case 1:   return "ERR_DEBUG_EXCEPTION";
        case 2:   return "ERR_NMI";
        case 3:   return "ERR_BREAKPOINT";
        case 4:   return "ERR_OVERFLOW";
        case 5:   return "ERR_BOUND_RANGE_EXCEEDED";
        case 6:   return "ERR_INVALID_OPCODE";
        case 7:   return "ERR_DEVICE_NOT_AVAILABLE";
        case 8:   return "ERR_DOUBLE_FAULT";
        case 9:   return "ERR_COPROCESSOR_OVERRUN";
        case 10:  return "ERR_INVALID_TSS";
        case 11:  return "ERR_SEGMENT_NOT_PRESENT";
        case 12:  return "ERR_STACK_SEGMENT_FAULT";
        case 13:  return "ERR_GENERAL_PROTECTION_FAULT";
        case 14:  return "ERR_PAGE_FAULT";
        case 16:  return "ERR_FPU_EXCEPTION";
        case 17:  return "ERR_ALIGNMENT_CHECK";
        case 18:  return "ERR_MACHINE_CHECK";
        case 19:  return "ERR_SIMD_EXCEPTION";
        case 20:  return "ERR_VIRTUALIZATION_EXCEPTION";
        case 21:  return "ERR_CONTROL_PROTECTION";
        case 28:  return "ERR_HYPERVISOR_INJECTION";
        case 29:  return "ERR_VMM_COMMUNICATION";
        case 30:  return "ERR_SECURITY_EXCEPTION";
        default:  return "ERR_UNKOWN";
    }
}

static const char *panic_describe(uint32_t int_no) {
    switch (int_no) {
       case 0:    return "Division by zero in arithmetic operation";
       case 1:    return "Debug exception triggered";
       case 2:    return "Non-maskable interrupt received";
       case 3:    return "Breakpoint instruction executed";
       case 4:    return "Overflow condition detected";
       case 5:    return "Array index out of bounds";
       case 6:    return "CPU encountered an illegal instruction";
       case 7:    return "FPU/SSE instruction on unavailable device";
       case 8:    return "Critical: exception while handling exception";
       case 12:   return "Stack operation caused segment violation";
       case 13:   return "Memory access violated protection rules";
       case 14:   return "Access to unmapped or protected memory page";
       case 16:   return "Floating point arithmetic error";
       case 17:   return "Unaligned memory access detected";
       case 18:   return "Critical hardware error detected";
       default:   return "An unexpected exception occurred";
    }
}

static addr_t read_cr2(void) {
    addr_t val;
#ifdef __x86_64__
    asm volatile ("mov %%cr2, %0" : "=r"(val));
#else
    asm volatile ("mov %%cr2, %0" : "=r"(val));
#endif
    return val;
}

static void panic_draw_button(uint32_t x, uint32_t y, const char *text, uint32_t bg) {
     uint32_t fw  = font_get_width();
     uint32_t fh  = font_get_height();
     int len = 0;
     while (text[len]) len++;

     uint32_t btn_w = (uint32_t)len * fw + 24;
     uint32_t btn_h = fh + 12;

     fb_fill_rect(x, y, btn_w, btn_h, bg);

     fb_fill_rect(x,              y,              btn_w, 1,     COLOR_WHITE);
     fb_fill_rect(x,              y + btn_h - 1,  btn_w, 1,     COLOR_WHITE);
     fb_fill_rect(x,              y,              1,     btn_h, COLOR_WHITE);
     fb_fill_rect(x + btn_w - 1, y,              1,     btn_h,  COLOR_WHITE);

     font_draw_string(x + 12, y + 6, text, COLOR_WHITE, bg, 1);
}

static void panic_draw_main(struct registers regs) {
   uint32_t w  = fb_get_width();
   uint32_t h  = fb_get_height();
   uint32_t fw = font_get_width();
   uint32_t fh = font_get_height();

   const char *code = panic_detect(regs.int_no);
   const char *desc = panic_describe(regs.int_no);

   fb_clear(COLOR_RED_BG);

   fb_fill_rect(0, 0, w, fh + 12, COLOR_YELLOW);
   font_draw_string_centered(6, "!!! VALIANTCORE PANIC !!!", COLOR_YELLOW_TXT, COLOR_YELLOW, 1);

   uint32_t y = fh + 32;
   font_draw_string_centered(y, "A critical error has been detected.", COLOR_WHITE, COLOR_RED_BG, 1);

   y += fh + 8;
   font_draw_string_centered(y, "The system has been halted to prevent damage.", COLOR_LIGHT, COLOR_RED_BG, 1);

   y += fh + 24;
   uint32_t box_x = w / 6;
   uint32_t box_w = w * 2 / 3;
   fb_fill_rect(box_x, y, box_w, fh * 6 + 24, 0xA93226);

   font_draw_string(box_x + 16, y + 8, "Error Code :", COLOR_YELLOW, 0xA93226, 1);
   font_draw_string(box_x + 16 + fw * 13, y + 8, code, COLOR_WHITE, 0xA93226, 1);

   font_draw_string(box_x + 16, y + 8 + fh + 4, "Description:", COLOR_YELLOW, 0xA93226, 1);
   font_draw_string(box_x + 16 + fw * 13, y + 8 + fh + 4, desc, COLOR_WHITE, 0xA93226, 1);

   char int_str[14] = "Exception #";
   int_str[11] = '0' + (regs.int_no / 10);
   int_str[12] = '0' + (regs.int_no % 10);
   int_str[13] = '\0';

   font_draw_string(box_x + 16, y + 8 + (fh + 4) * 2, "Exception :", COLOR_YELLOW, 0xA93226, 1);
   font_draw_string(box_x + 16 + fw * 13, y + 8 + (fh + 4) * 2, int_str, COLOR_WHITE, 0xA93226, 1);

   font_draw_string(box_x + 16, y + 8 + (fh + 4) * 3, "Architecture:", COLOR_YELLOW, 0xA93226, 1);

#ifdef __x86_64__
   font_draw_string(box_x + 16 + fw * 13, y + 8 + (fh + 4) * 3, "x86_64", COLOR_WHITE, 0xA93226, 1);
#else
   font_draw_string(box_x + 16 + fw * 13, y + 8 + (fh + 4) * 3, "i386", COLOR_WHITE, 0xA93226, 1);
#endif

   if (regs.int_no == 14) {
         char cr2_str[20];
         uint64_to_hex((uint64_t)read_cr2(), cr2_str);
         font_draw_string(box_x + 16, y + 8 + (fh + 4) * 4, "Fault Addr :", COLOR_YELLOW, 0xA93226, 1);
         font_draw_string(box_x + 16 + fw * 13, y + 8 + (fh + 4) * 4, cr2_str, COLOR_CYAN, 0xA93226, 1);
   }

   y += fh * 6 + 24 + 20;
   uint32_t btn_x = (w - fw * 26 - 24) / 2;
   panic_draw_button(btn_x, y, "[D] Show Full Diagnostic Report", COLOR_BLUE_BTN);

   y += fh + 12 + 20;
   font_draw_string_centered(y, "Cannot resolve this issue? Open an issue at:", COLOR_LIGHT, COLOR_RED_BG, 1);

   y += fh + 6;
   font_draw_string_centered(y, "github.com/finndev62/valiantcore -> Issue tab", COLOR_YELLOW, COLOR_RED_BG, 1);

   fb_fill_rect(0, h - fh - 12, w, fh + 12, COLOR_YELLOW);
   font_draw_string_centered(h - fh - 6, "ValiantCore Kernel — Press [D] for full details", COLOR_YELLOW_TXT, COLOR_YELLOW, 1);
}

static void panic_draw_detail(struct registers regs) {
      uint32_t w  = fb_get_width();
      uint32_t h  = fb_get_height();
      uint32_t fw = font_get_width();
      uint32_t fh = font_get_height();

      fb_clear(COLOR_BLUE_BG);


      fb_fill_rect(0, 0, w, fh + 12, 0x0D47A1);
      font_draw_string_centered(6, "VALIANTCORE FULL DIAGNOSTIC REPORT", COLOR_WHITE, 0x0D47A1, 1);


      uint32_t y  = fh + 24;
      uint32_t x  = 40;
      uint32_t lh = fh + 6;

      font_draw_string(x, y, "[ EXCEPTION]", COLOR_CYAN, COLOR_BLUE_BG, 1);

      y += lh + 4;

      char int_buf[4] = {'#', '0' + (regs.int_no/10), '0' + (regs.int_no%10), '\0'};
      font_draw_string(x, y, " Exception", COLOR_BLUE_LIGHT, COLOR_BLUE_BG, 1);
      font_draw_string(x + fw * 16, y, int_buf, COLOR_WHITE, COLOR_BLUE_BG, 1);
      y += lh;


      font_draw_string(x, y, " Error Code :", COLOR_BLUE_LIGHT, COLOR_BLUE_BG, 1);
      font_draw_string(x + fw * 16, y, panic_detect(regs.int_no), COLOR_YELLOW, COLOR_BLUE_BG, 1);


      y += lh;

      font_draw_string(x, y, " Description:", COLOR_BLUE_LIGHT, COLOR_BLUE_BG, 1);
      font_draw_string(x + fw * 16, y, panic_describe(regs.int_no), COLOR_WHITE, COLOR_BLUE_BG, 1);

      y += lh + 8;

      font_draw_string(x, y, "[ REGISTER DUMP ]", COLOR_CYAN, COLOR_BLUE_BG, 1);


      y += lh + 4;
#ifdef __x86_64__
    char rip[20], rsp[20], rbp[20], rax[20], rbx[20], rcx[20], rdx[20];
    uint64_to_hex(regs.rip,   rip);
    uint64_to_hex(regs.rsp,   rsp);
    uint64_to_hex(regs.rbp,   rbp);
    uint64_to_hex(regs.rax,   rax);
    uint64_to_hex(regs.rbx,   rbx);
    uint64_to_hex(regs.rcx,   rcx);
    uint64_to_hex(regs.rdx,   rdx);

    font_draw_string(x,           y, " RIP:", COLOR_BLUE_LIGHT,  COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 8,  y, rip,  COLOR_GREEN,          COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 28, y, " RSP:", COLOR_BLUE_LIGHT,  COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 34, y, rsp,  COLOR_GREEN,          COLOR_BLUE_BG, 1);
    y += lh;
    font_draw_string(x,           y, " RBP:", COLOR_BLUE_LIGHT,   COLOR_BLUE_BG,  1);
    font_draw_string(x + fw * 8,  y, rbp,       COLOR_GREEN,      COLOR_BLUE_BG,  1);
    font_draw_string(x + fw * 28, y,  " RAX:", COLOR_BLUE_LIGHT,  COLOR_BLUE_BG,  1);
    font_draw_string(x + fw * 34, y, rax,       COLOR_GREEN,      COLOR_BLUE_BG,  1);
    y += lh;
    font_draw_string(x,           y, " EBX:", COLOR_BLUE_LIGHT,  COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 8,  y, rbx,    COLOR_GREEN,        COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 20, y, " ECX:", COLOR_BLUE_LIGHT,  COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 26, y, rcx,    COLOR_GREEN,        COLOR_BLUE_BG, 1);

    font_draw_string(x,          y, "  EDX:", COLOR_BLUE_LIGHT, COLOR_BLUE_BG, 1);
    font_draw_string(x + fw * 8, y, rdx,     COLOR_GREEN,       COLOR_BLUE_BG, 1);
    y += lh + 8;
#endif

if (regs.int_no == 14) {
   font_draw_string(x, y, "[FAULT ADDRESS]", COLOR_CYAN, COLOR_BLUE_BG, 1);


   y += lh +4;


   char cr2_str[20];
   uint64_to_hex((uint64_t)read_cr2(), cr2_str);
   font_draw_string(x, y, cr2_str, COLOR_WHITE, COLOR_BLUE_BG, 1);

   y += lh + 8;

   char err_hex[12];
   uint32_to_hex(regs.err_code, err_hex);
   font_draw_string(x,          y, " Error Code (raw):", COLOR_BLUE_LIGHT, COLOR_BLUE_BG, 1);
   font_draw_string(x + fw * 20,y, err_hex, COLOR_WHITE, COLOR_BLUE_BG, 1);
   y += lh + 8;

   fb_fill_rect(0, h - fh - 12, w, fh + 12, 0x0D47A1);
   font_draw_string_centered(h - fh - 6, "github.com/finndev62/valiantcore -> Issues tab", COLOR_WHITE, 0x0D47A1, 1);
}
}
 /* kernel_panic */
 void kernel_panic(struct registers regs) {
       asm volatile ("cli");


        if (!fb_is_ready()) {
            for (;;) asm volatile ("hlt");
        }

        panic_draw_main(regs);


        while (1) {
            if (keyboard_has_input()) {
                char c = keyboard_getchar();
                if (c == 'd' || c == 'D') {
                    panic_draw_detail(regs);
                }
            }
            asm volatile ("hlt");
        }
}
    
    
    
    
    
    
  
    
    
   

     
