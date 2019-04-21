#include "defs.h"
#include "cpu.h"
#include "syscall.h"

extern struct cpu_t cpu;

int sys_fork(void);

static int (*syscalls[])(void) = {
[SYS_fork] = sys_fork,
};

int sys_fork() {
    return fork();
}

void syscall(void) {
    static char *states[] = {
        [SYS_fork]    "fork",
    };
    struct proc_t *curproc = cpu.proc;
    int num = curproc->tf->eax;
    if (num == 1) {
        vga_console_printf("syscall %d %s\n", num, states[num]);
        curproc->tf->eax = syscalls[num]();
    } else {
        vga_console_printf("fork returned %d, %s process\n", num, num == 0 ? "child" : "parent");
    }
}