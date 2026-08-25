/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "../include/kernel.h"
#include "../include/scheduler.h"
#include "../include/font.h"
#include "../include/framebuffer.h"
#include "../include/terminus16.h"
#include "../include/vcore_logo.h"


extern void vfs_init();
extern int rtl8111_init();
extern void net_init();


void kmain(uint64_t fb_base, uint32_t fb_width,
           uint32_t fb_height, uint32_t fb_pitch) {

   init_gdt();     // By Finn Dev
   pic_init();     // 
   pit_init(1000); // 
   init_idt();     //
   init_scheduler(); 

   

   monitor_system_integrity();
   
   fat32_init();
   vfs_init();

   

font_init(Uni2_Terminus16_psf);   
font_set_color(0xFFFFFF, 0x0A0A2A);  
fb_clear(0x0A0A2A);

boot_screen_show();

for (int i = 0; i < 37; i++) {
    boot_screen_spin();
    pit_sleep(80);
}

fb_clear(COLOR_BG);
font_set_cursor(0, 0);
kprint("ValiantCore ready.\n");

   net_init();
   if (rtl8111_init() == 0) {
      kprint("[KERNEL] RTL8111 driver loaded.\n");
          } else {
              kprint("[KERNEL] ERROR: RTL8111 driver failed!\n");
          }

          kprint("[KERNEL] ValiantCore ready.\n");


if (fb_tag) {
    fb_init(fb_base, fb_pitch, fb_width, fb_height, 32);
}

   while (1);
              }

