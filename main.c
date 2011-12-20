#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sh4.h"

extern struct cpu *cpu;       // This cpu.
extern struct proc *proc;     // Current proc on this cpu.

static void mpmain(void);
void jkstack(void)  __attribute__((noreturn));
void mainc(void);

unsigned char xv6_stack[STACK_SIZE];

extern unsigned char __bss_start;
extern unsigned char __bss_end;

// Bootstrap processor starts running C code here.
int
main(void)
{
  tvinit();        // trap vectors
  scif_init();     // serial port
  ksegment();      // set up segments
  consoleinit();   // I/O devices & their interrupts
  kinit();         // initialize memory allocator
  jkstack();       // call mainc() on a properly-allocated stack 
}

void
jkstack(void)
{
  char *kstack = kalloc();
  if(!kstack)
    panic("jkstack\n");
  char *top = kstack + PGSIZE;
#ifdef DEBUG
  cprintf("%s: kstack=0x%x\n", __func__, kstack);
  cprintf("%s: top=0x%x\n", __func__, top);
#endif
  asm volatile(
      "mov %0, r15\n"
      :
      : "r"(top)
      );
  mainc(); 
  panic("jkstack");
}

void
mainc(void)
{
  cprintf("\ncpu%d: starting xv6\n\n", cpu->id);
  kvmalloc();      // initialize the kernel page table
  pinit();         // process table
  binit();         // buffer cache
  fileinit();      // file table
  iinit();         // inode cache
  ideinit();       // disk
  timer_init();    // uniprocessor timer
  userinit();      // first user process
#ifdef DEBUG
  cprintf("%s: userinit done\n", __func__);
#endif

  // Finish setting up this processor in mpmain.
  mpmain();
}

// Common CPU setup code.
// Bootstrap CPU comes here from mainc().
// Other CPUs jump here from bootother.S.
static void
mpmain(void)
{
  vmenable();         // turn on paging
  cprintf("cpu%d: starting\n", cpu->id);
  cpu->booted = 1;
  scheduler();        // start running processes
}

#if 0
static void
bootothers(void)
{
  extern uchar _binary_bootother_start[], _binary_bootother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write bootstrap code to unused memory at 0x7000.  The linker has
  // placed the start of bootother.S there.
  code = (uchar *) 0x7000;
  memmove(code, _binary_bootother_start, (uint)_binary_bootother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == cpus+cpunum())  // We've started already.
      continue;

    // Fill in %esp, %eip and start code on cpu.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void**)(code-8) = mpmain;
    lapicstartap(c->id, (uint)code);

    // Wait for cpu to finish mpmain()
    while(c->booted == 0)
      ;
  }
}
#endif
