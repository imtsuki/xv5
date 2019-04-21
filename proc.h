#ifndef PROC_H
#define PROC_H

#include "include/types.h"
#include "traps.h"

#define NPROC 64

struct context_t {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebx;
  uint32_t ebp;
  uint32_t eip;
};

enum procstate { UNUSED, BORN, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc_t {
  size_t sz;
  pde_t* pgdir;                // 进程页表
  uint8_t *kstack;             // 该进程对应的内核栈
  enum procstate state;        // 进程状态
  struct trapframe *tf;
  struct context_t *context;     // 进程上下文
  int pid;
};

#endif