#include "include/mmu.h"
#include "include/types.h"

struct cpu_t {
    struct gatedesc gdt[NSEGS];
};

struct proc_t {
    pde_t *pde;
};

struct cpu_t cpu;