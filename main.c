#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sh4.h"

static void bootothers(void);
static void mpmain(void);
void jmpkstack(void)  __attribute__((noreturn));
void mainc(void);
unsigned char xv6_stack[STACK_SIZE];

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  mpinit();        // collect info about this machine
  tvinit();        // trap vectors
  scif_init();     // serial port
  seginit();       // set up segments
  kinit();         // initialize memory allocator
  jmpkstack();       // call mainc() on a properly-allocated stack 
}

void
jmpkstack(void)
{
  char *kstack, *top;
  
  kstack = kalloc();
  if(kstack == 0)
    panic("jmpkstack kalloc");
  top = kstack + PGSIZE;
#if 0
  asm volatile("movl %0,%%esp; call mainc" : : "r" (top));
#else
#ifdef DEBUG
  cprintf("%s: kstack=0x%x\n", __func__, kstack);
  cprintf("%s: top=0x%x\n", __func__, top);
#endif
  asm volatile("mov %0, r15\n" : : "r"(top));
  mainc(); 
#endif
  panic("jmpkstack");
}

// Set up hardware and software.
// Runs only on the boostrap processor.
void
mainc(void)
{
  cprintf("\ncpu%d: starting xv6\n\n", cpu->id);
  consoleinit();   // I/O devices & their interrupts
  kvmalloc();      // initialize the kernel page table
  pinit();         // process table
  binit();         // buffer cache
  fileinit();      // file table
  iinit();         // inode cache
  ideinit();       // disk
#if 0
  if(!ismp)
    timerinit();   // uniprocessor timer
#else
  timer_init();    // uniprocessor timer
#endif
  userinit();      // first user process
  bootothers();    // start other processors
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
  vmenable();        // turn on paging
  cprintf("cpu%d: starting\n", cpu->id);
  cpu->booted = 1;

  scheduler();     // start running processes
}

// Start the non-boot processors.
static void
bootothers(void)
{
#if 0
  extern uchar _binary_bootother_start[], _binary_bootother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write bootstrap code to unused memory at 0x7000.
  // The linker has placed the image of bootother.S in
  // _binary_bootother_start.
  code = (uchar*)0x7000;
  memmove(code, _binary_bootother_start, (uint)_binary_bootother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == cpus+cpunum())  // We've started already.
      continue;

    // Tell bootother.S what stack to use and the address of mpmain;
    // it expects to find these two addresses stored just before
    // its first instruction.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void**)(code-8) = mpmain;

    lapicstartap(c->id, (uint)code);

    // Wait for cpu to finish mpmain()
    while(c->booted == 0)
      ;
  }
#endif
}

//PAGEBREAK!
// Blank page.

