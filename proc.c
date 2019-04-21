#include "proc.h"
#include "defs.h"
#include "cpu.h"
#include "include/mmu.h"
#include "include/types.h"
#include "include/string.h"
#include "include/x86.h"

struct cpu_t cpu;

extern void trapret(void);
extern void forkret(void);

int nextpid = 1;

struct {
    struct proc_t proc[NPROC];
} ptable;

struct proc_t *allocproc(void) {
    for (struct proc_t *p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED) {
            // Process state
            p->state = BORN;
            // Allocate kernel stack
            if ((p->kstack = kalloc()) == 0) {

                p->state = UNUSED;
                return 0;
            }
            p->pid = nextpid++;
            uint8_t *sp = p->kstack + KSTACKSIZE;
            sp -= sizeof(*(p->tf));
            p->tf = (struct trapframe *)sp;
            sp -= 4;
            *(uint32_t *)sp = (uint32_t)trapret;
            sp -= sizeof(*(p->context));
            p->context = (struct context_t *)sp;
            memset(p->context, 0,  sizeof(*(p->context)));
            p->context->eip = (uint32_t)forkret;
            return p;
        }
    }
    return 0;
}

void forkret() {
    return;
}

void user_init() {
    extern char _binary_initcode_start[], _binary_initcode_size[];
    struct proc_t *p;
    
    for (struct proc_t *p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        p->state = UNUSED;
    }

    p = allocproc();

    p->pgdir = setupkvm();

    inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
    memset(p->tf, 0, sizeof(*p->tf));

    p->sz = PGSIZE;
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;
    p->state = RUNNABLE;
}

void run_scheduler(void) {

    struct proc_t *p;
    cpu.proc = 0;
    while (1) {
        sti();
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->state != RUNNABLE)
                continue;
        cpu.proc = p;
        switchuvm(p);
        p->state = RUNNING;
        //vga_console_printf("%x\n", *(uint16_t *)0);
        swtch(&(cpu.sched_context), p->context);
        switchkvm();
        

        cpu.proc = 0;
        }
    }
}

void return_to_scheduler() {
    swtch(&cpu.proc->context, cpu.sched_context);
}

void yield(void) {
    cpu.proc->state = RUNNABLE;
    return_to_scheduler();
}

void exit(void) {
    cpu.proc->state = ZOMBIE;
    return_to_scheduler();
}

int fork(void) {
    struct proc_t *child;
    struct proc_t *curproc = cpu.proc;

    // Allocate process.
    if((child = allocproc()) == 0){
        return -1;
    }

  // Copy process state from proc.
    if((child->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
        kfree(child->kstack);
        child->kstack = 0;
        child->state = UNUSED;
        return -1;
    }
    child->sz = curproc->sz;
    *child->tf = *curproc->tf;

    // Clear %eax so that fork returns 0 in the child.
    child->tf->eax = 0;
    child->state = RUNNABLE;
    return child->pid;
}