/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/
#include "../include/kernel.h"
#include <stdint.h>


typedef struct __attribute__ ((packed)) {
   char  magic[4];
   uint8_t arch;
   uint32_t entry_point;
   uint32_t text_size;
} vef_header_t;

#define VEF_ARCH_I386     1
#define VEF_ARCH_X86_64   2 
#define VEF_ARCH_AARCH64  3


#define WIN_BG         0x000000
#define WIN_BORDER     0x444444
#define WIN_TITLE_BG   0x1A1A2E
#define WIN_FG         0x00FF00
#define WIN_ERR        0xFF4444
#define WIN_TITLE_FG   0xCCCCCC





static struct {
     uint32_t x, y, w, h;
     uint32_t text_x, text_y;
     uint32_t text_start_y;
     int      active;
} vef_win;

/* vef_win_open */
static void vef_win_open(const char *title) {
   uint32_t sw = fb_get_width();
   uint32_t sh = fb_get_height();
   uint32_t fw = font_get_width();
   uint32_t fh = font_get_height();


   vef_win.w = 600;
   vef_win.h = 400;
   vef_win.x = (sw - vef_win.w) / 2;
   vef_win.y = (sh - vef_win.h) / 2;

   /* Outer frame is grey */
   fb_fill_rect(vef_win.x - 2, vef_win.y - 2, vef_win.w + 4, vef_win.h + 4, WIN_BORDER);
   /* Header bar */
   fb_fill_rect(vef_win.x, vef_win.y, vef_win.w, fh + 10, WIN_TITLE_BG);

   /*  Header Text */
   font_draw_string(vef_win.x + 8, vef_win.y + 5, title, WIN_TITLE_FG, WIN_TITLE_BG, 1);
   /* Black content area */
   fb_fill_rect(vef_win.x, vef_win.y + fh + 10, vef_win.w, vef_win.h - fh - 10, WIN_BG);
   

/* text starting positions */
vef_win.text_x       = vef_win.x + 8;
vef_win.text_y       = vef_win.y + fh + 18;
vef_win.text_start_y = vef_win.text_y;
vef_win.active       = 1;

}
/* vef_win_print */
static void vef_win_print(const char *s, uint32_t color) {
   if (!vef_win.active || !s) return;

   uint32_t fw = font_get_width();
   uint32_t fh = font_get_height();
   uint32_t max_x = vef_win.x + vef_win.w - 8;
   uint32_t max_y = vef_win.y + vef_win.h - fh - 4;

   while (*s) {
       if (*s == '\n' || vef_win.text_x + fw > max_x) {
          vef_win.text_x = vef_win.x + 8;
          vef_win.text_y += fh + 2;

          if (vef_win.text_y > max_y) {

             fb_fill_rect(vef_win.x, vef_win.y + fh + 10, vef_win.w, vef_win.h + fh - 10, WIN_BG);
             vef_win.text_y = vef_win.text_start_y;
           }

           if (*s == '\n') { s++; continue; }
         }

         char ch[2] = { *s, '\0' };
         font_draw_string(vef_win.text_x, vef_win.text_y, ch, color, WIN_BG, 1);
         vef_win.text_x += fw;
         s++;
    }
}
                  
/* vef_win_close */
static void vef_win_close(void) {
    fb_fill_rect(vef_win.x - 2, vef_win.y - 2, vef_win.w + 4, vef_win.h + 4, 0x000000);
                 vef_win.active = 0;
}
/* vef_current_arch */
static uint8_t vef_current_arch(void) {
#if defined(__aarch64__)
    return VEF_ARCH_AARCH64;
#elif defined(__x86_64__)
    return VEF_ARCH_X86_64;
#else
    return VEF_ARCH_I386;
#endif
}
/* jump_to_ring3 */
static void jump_to_ring3(uint32_t entry, uint32_t user_stack) {
#ifdef __x86_64__ 

     asm volatile (
         "mov $0x23, %%ax\n"
         "mov %%ax,  %%ds\n"
         "mov %%ax,  %%es\n"
         "mov %%ax,  %%fs\n"
         "mov %%ax,  %%gs\n"
         "pushq $0x23\n"
         "pushq %1\n"
         "pushfq\n"
         "pushq $0x1B\n"
         "pushq %0\n"
         "iretq\n"
         :: "r"((uint64_t)entry), "r"((uint64_t)user_stack)
         : "ax"
      );
#else
    asm volatile (
        "mov $0x23, %%ax\n"
        "mov %%ax,  %%ds\n"
        "mov %%ax,  %%es\n"
        "mov %%ax,  %%fs\n"
        "mov %%ax,  %%gs\n"
        "pushl $0x23\n"
        "pushl %1\n"
        "pushfl\n"
        "pushl $0x1B\n"
        "pushl %0\n"
        "iret\n"
        :: "r"(entry), "r"(user_stack)
        :  "ax"
     );
#endif
}


/* User stack area */
static uint8_t user_stack[4096] __attribute__((aligned(4096)));


/* vef_exec */
int vef_exec(uint8_t *buf, uint64_t size) {
    if (!buf || size < sizeof(vef_header_t)) {
       kprint("ERROR: Invalid VEF buffer\n");
       return -1;
}

vef_header_t *hdr = (vef_header_t *)buf;


if (hdr->magic[0] != 'F' || hdr->magic[1] != 'I' || hdr->magic[2] != 'N' || hdr->magic[3] != 'N') {
vef_win_open("VEF Loader - Error");
vef_win_print("ERROR: No valid signature was found in this file.\n", WIN_ERR);
vef_win_print("Expected: FINN signature\n", WIN_ERR);
vef_win_print("This file cannot be executed.\n", WIN_ERR);

return -1;
}


/*——————————————ARCH CONTROL————————————————————————————————————————————————————————*/
 uint8_t cur_arch = vef_current_arch();
 if (hdr->arch != cur_arch) {
   vef_win_open("VEF Loader - Error");
   vef_win_print("ERROR: Architecture mismatch.\n", WIN_ERR);
   if (hdr->arch == VEF_ARCH_I386)
      vef_win_print("File is for i386, current system is different.\n", WIN_ERR);
  else if (hdr->arch == VEF_ARCH_X86_64)
            vef_win_print("File is for x86_64, current system is different.\n", WIN_ERR);
  else if (hdr->arch == VEF_ARCH_AARCH64)
             vef_win_print("File is for Aarch64, current system is different.\n", WIN_ERR);
          return -1;
       }


/* ———————————————DIMENSION CONTROL—————————*/
if ((uint64_t)hdr->text_size + sizeof(vef_header_t) > size) {
    vef_win_open("VEF Loader - Error");
    vef_win_print("ERROR: Corrupted VEF file (size mismatch).\n", WIN_ERR);
    return -1;
}



/* ——————————————SECURITY SCAN———————————————*/
const uint8_t *payload = buf + sizeof(vef_header_t);
if (security_scan_buffer(payload, hdr->text_size) != 0) {
   vef_win_open("VEF Loader - Security");
   vef_win_print("ERROR: Security scan failed.\n", WIN_ERR);
   vef_win_print("Execution blocked by ValiantCore's Cyber Armor.\n", WIN_ERR);
   return -1;
}


/*-----------------------------------*/
vef_win_open("ValiantCore VEF - Running");
vef_win_print("VEF signature: OK\n", WIN_FG);
vef_win_print("Security scan: OK\n", WIN_FG);
vef_win_print("Loading...\n\n", WIN_FG);


/*--------entry_point------*/
uint8_t *load_addr = (uint8_t *)(addr_t)hdr->entry_point;
for (uint32_t i = 0; i < hdr->text_size; i++) {
    load_addr[i] = payload[i];
}


/* Paging Ring 3 */
paging_map_user((uint64_t)hdr->entry_point,
                (uint64_t)hdr->entry_point,
                (uint64_t)hdr->text_size);


/*------------------*/
uint32_t stack_top = (uint32_t)((addr_t)user_stack + sizeof(user_stack));
paging_switch_user();
jump_to_ring3(hdr->entry_point, stack_top);


paging_switch_kernel();
vef_win_close();

return 0;

}
/* ValiantCore vef-loader.c file end of line It's finally over :D */
/* The End */
  
