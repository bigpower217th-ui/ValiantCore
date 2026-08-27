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

   init_gdt();
   pic_init();
   pit_init(1000);
   init_idt();
   init_scheduler();

   monitor_system_integrity();
   pci_init();
   fat32_init();
   vfs_init();

font_init(Uni2_Terminus16_psf);
font_set_color(0xFFFFFF, 0x0A0A2A);
fb_clear(0x0A0A2A);

kprint("ValiantCore ready.\n");

   net_init();
   if (rtl8111_init() == 0) {
      kprint("[KERNEL] RTL8111 driver loaded.\n");
          } else {
              kprint("[KERNEL] ERROR: RTL8111 driver failed!\n");
          }

          kprint("[KERNEL] ValiantCore ready.\n");

   fb_init(fb_base, fb_pitch, fb_width, fb_height, 32);

   while (1);
              }
