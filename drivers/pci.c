/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/
#include "../include/kernel.h"
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define PCI_MAX_BUS        256
#define PCI_MAX_SLOT       32
#define PCI_MAX_FUNC       8
#define PCI_MAX_DEVICES    256


#define PCI_VENDOR_ID      0x0
#define PCI_DEVICE_ID      0x02
#define PCI_COMMAND        0x04
#define PCI_STATUS         0x06
#define PCI_CLASS_CODE     0x0B
#define PCI_SUBCLASS       0x0A
#define PCI_PROG_IF        0x09
#define PCI_HEADER_TYPE    0x0E
#define PCI_BAR0           0x10
#define PCI_BAR1           0x14
#define PCI_BAR2           0x18
#define PCI_BAR3           0x1C
#define PCI_BAR4           0x20
#define PCI_BAR5           0x24
#define PCI_IRQ_LINE       0x3C


#define PCI_CLASS_STORAGE  0x01
#define PCI_CLASS_NETWORK  0x02
#define PCI_CLASS_DISPLAY  0x03
#define PCI_CLASS_BRIDGE   0x06

#define PCI_SUB_IDE        0x01
#define PCI_SUB_SATA       0x06
#define PCI_SUB_ETHERNET   0x00
#define PCI_SUB_VGA        0x00


#define VENDOR_REALTEK   0x10EC
#define DEVICE_RTL8111   0x8168
#define VENDOR_INTEL     0x8086
#define VENDOR_AMD       0x1022


/* PCI Device structure */
typedef struct {
    uint8_t   bus;
    uint8_t   slot;
    uint8_t   func;
    uint16_t  vendor_id;
    uint16_t  device_id;
    uint8_t   class_code;
    uint8_t   subclass;
    uint8_t   prog_if;
    uint8_t   irq;
    uint32_t bar[6];
    int      valid;
} pci_device_t;

static pci_device_t pci_devices[PCI_MAX_DEVICES];
static uint32_t     pci_device_count = 0;


/* pci_read32 —— PCI_CONFIG */
static uint32_t pci_read32(uint8_t bus, uint32_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1UL << 31)
                     | ((uint32_t)bus   << 16)
                     | ((uint32_t)slot  << 11)
                     | ((uint32_t)func  << 8)
                     | (offset & 0xFC);
    outb(PCI_CONFIG_ADDRESS,     (address >> 0)  & 0xFF);
    outb(PCI_CONFIG_ADDRESS + 1, (address >> 8)  & 0xFF);
    outb(PCI_CONFIG_ADDRESS + 2, (address >> 16) & 0xFF);
    outb(PCI_CONFIG_ADDRESS + 3, (address >> 24) & 0xFF);
    return inb(PCI_CONFIG_DATA)
         | ((uint32_t)inb(PCI_CONFIG_DATA + 1) << 8)
         | ((uint32_t)inb(PCI_CONFIG_DATA + 2) << 16)
         | ((uint32_t)inb(PCI_CONFIG_DATA + 3) << 24);
}

static uint16_t pci_read16(uint16_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
   uint32_t val = pci_read32(bus, slot, func, offset & ~3);
   return (val >> ((offset & 3) * 8)) & 0xFFFF;
}

static uint8_t pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
   uint32_t val = pci_read32(bus, slot, func, offset & ~3);
   return (val >> ((offset & 3) * 8)) & 0xFF;
}


/* pci_write32 */
static void pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
   uint32_t address = (1UL << 31)
                    | ((uint32_t)bus   << 16)
                    | ((uint32_t)slot  << 11)
                    | ((uint32_t)func  << 8)
                    | (offset & 0xFC);
   outb(PCI_CONFIG_ADDRESS,     (address >> 0)  & 0xFF);
   outb(PCI_CONFIG_ADDRESS + 1, (address >> 8)  & 0xFF);
   outb(PCI_CONFIG_ADDRESS + 2, (address >> 16) & 0xFF);
   outb(PCI_CONFIG_ADDRESS + 3, (address >> 24) & 0xFF);
   outb(PCI_CONFIG_DATA,     (val >> 0)  & 0xFF);
   outb(PCI_CONFIG_DATA + 1, (val >> 8)  & 0xFF);
   outb(PCI_CONFIG_DATA + 2, (val >> 16) & 0xFF);
   outb(PCI_CONFIG_DATA + 3, (val >> 24) & 0xFF);
}
/* pci_scan_device */
static void pci_scan_device(uint8_t bus, uint8_t slot, uint8_t func) {
    uint16_t vendor = pci_read16(bus, slot, func, PCI_VENDOR_ID);
    if (vendor == 0xFFFF) return;


    if (pci_device_count >= PCI_MAX_DEVICES) return;


    pci_device_t *dev = &pci_devices[pci_device_count++];
    dev->bus        = bus;
    dev->slot       = slot;
    dev->func       = func;
    dev->vendor_id  = vendor;
    dev->device_id  = pci_read16(bus, slot, func, PCI_DEVICE_ID);
    dev->class_code = pci_read8(bus, slot, func, PCI_CLASS_CODE);
    dev->subclass   = pci_read8(bus, slot, func, PCI_SUBCLASS);
    dev->prog_if    = pci_read8(bus, slot, func, PCI_PROG_IF);
    dev->irq        = pci_read8 (bus, slot, func, PCI_IRQ_LINE);
    dev->valid      = 1;


    for (int i = 0; i < 6; i++) 
        dev->bar[i] = pci_read32(bus, slot, func, PCI_BAR0 + i * 4);


        uint32_t cmd = pci_read16(bus, slot, func, PCI_COMMAND);
        cmd |= 0x0006;
        pci_write32(bus, slot, func, PCI_COMMAND, cmd);
   }




   /* pci_init */
   void pci_init(void) {
       pci_device_count = 0;


       for (uint16_t bus = 0; bus < PCI_MAX_BUS; bus++) {
           for (uint8_t slot = 0; slot < PCI_MAX_SLOT; slot++) {
               uint16_t vendor = pci_read16(bus, slot, 0, PCI_VENDOR_ID);
               if (vendor == 0xFFFF) continue;

               pci_scan_device(bus, slot, 0);
 

               uint8_t hdr = pci_read8(bus, slot, 0, PCI_HEADER_TYPE);
               if (hdr & 0x80) {
                  for (uint8_t func = 1; func < PCI_MAX_FUNC; func++) {
                      vendor = pci_read16(bus, slot, func, PCI_VENDOR_ID);
                      if (vendor != 0xFFFF)
                         pci_scan_device(bus, slot, func);
                     }
               }
        }
}
kprint("[PCI] Scan complete. Devices found: ");
char buf[8];
uint32_t n = pci_device_count;
int pos = 0;
if (n == 0) buf[pos++] = '0';
else {
      char tmp[8]; int tp = 0;
      while (n > 0) { tmp[tp++] = '0' + (n % 10); n /= 10; }
      for (int i = tp-1; i >= 0; i--) buf[pos++] = tmp[i];
   }
   buf[pos] = '\0';
   kprint(buf);
   kprint("\n");

   for (uint32_t i = 0; i < pci_device_count; i++) {
       pci_device_t *d = &pci_devices[i];


       if (d->vendor_id == VENDOR_REALTEK && d->device_id == DEVICE_RTL8111)
          kprint("[PCI] Found: Realtek RTL8111 Ethernet\n");
       else if (d->class_code == PCI_CLASS_STORAGE && d->subclass   == PCI_SUB_SATA)
          kprint("[PCI] Found: SATA Controller\n");
       else if (d->class_code == PCI_CLASS_STORAGE && d->subclass   == PCI_SUB_IDE)
          kprint("[PCI] Found: IDE Controller\n");
       else if (d->class_code == PCI_CLASS_DISPLAY)
          kprint("[PCI] Found: Display Controller\n");
       else if (d->class_code == PCI_CLASS_NETWORK)
          kprint("[PCI] Found: Network Controller\n");
      }
}
/* pci_find_device */
pci_device_t *pci_find_device(uint16_t vendor, uint16_t device) {
    for (uint32_t i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].vendor_id == vendor && pci_devices[i].device_id == device)
           return &pci_devices[i];
        }
       return (void *)0;
}

pci_device_t* pci_find_class(uint8_t class_code, uint8_t subclass) {
   for (uint32_t i = 0; i < pci_device_count; i++) {
          if (pci_devices[i].class_code == class_code && pci_devices[i].subclass == subclass)
          return &pci_devices[i];
     }
     return (void *)0;
}

/* pci_get_bar_addres */
addr_t pci_get_bar_addres(pci_device_t *dev, int bar_idx) {
     if (!dev || bar_idx < 0 || bar_idx > 5) return 0;
     uint32_t bar = dev->bar[bar_idx];

     /* Sorry, I put the code in the trash, but what can I do, that's just how I write code. */
     if (bar & 1) {

       return (addr_t)(bar & ~3);
    } else {
       return (addr_t)(bar & ~15);
    }
}
