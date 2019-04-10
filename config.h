#ifndef CONFIG_H
#define CONFIG_H

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTOP 0xE000000           // Top physical memory
#define DEVSPACE 0xFD000000         // Other devices are at high addresses

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
#define KERNLINK (KERNBASE + EXTMEM)  // Address where kernel is linked
#define KSTACKSIZE 4096  // size of per-process kernel stack

#endif // CONFIG_H