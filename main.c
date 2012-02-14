#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "sh4.h"

unsigned char xv6_stack[STACK_SIZE];
unsigned char *xv6_smp_stack;
static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  kvmalloc();      // kernel page table
  mpinit();        // collect info about this machine
  tvinit();        // trap vectors
  scif_init();     // serial port
  seginit();       // set up segments
  cprintf("\ncpu%d: starting xv6\n\n", cpu->id);
  consoleinit();   // I/O devices & their interrupts
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
  startothers();    // start other processors (must come before kinit)
  kinit();         // initialize memory allocator
  userinit();      // first user process  (must come after kinit)
  // Finish setting up this processor in mpmain.
  mpmain();
}

// Other CPUs jump here from entryother.S.
void
mpenter(void)
{
  //cprintf("cpu%d: mpenter\n", cpu->id);
  while(1);
#if 0
  switchkvm(); 
  seginit();
  lapicinit(cpunum());
  mpmain();
#endif
}

#define RESETVEC_BASE 0xFE400008
#define STBCR_BASE 0xFE400004
#define STBCR_MSTP  0x00000001
#define STBCR_RESET 0x00000002
#define STBCR_LTSLP 0x80000000
#define STBCR_AP_VAL (STBCR_RESET | STBCR_LTSLP)

extern void smp_start(void);
void slave_boot(unsigned int start_pc, int id)
{
  volatile unsigned int *resetvec = (RESETVEC_BASE + (id << 12));
  volatile unsigned int *stbcr = (STBCR_BASE + (id <<12 ));

  /* Set RESETVEC to start from entry point of slave */
  *resetvec = start_pc;
  cprintf("RESETVEC%d on %x\n", id, resetvec);
  cprintf("Setting RESETVEC%d to %x\n", id, start_pc);
  while( *resetvec != start_pc);
  cprintf("Succeeded to set RESETVEC%d to specified value\n", id);

  /* Send reset and boot CPU from RESETVEC */
  cprintf("Sending reset to CPU%d\n", id);

  if (!(*stbcr & STBCR_MSTP))
    *stbcr |= STBCR_MSTP;

  while (!(*stbcr & STBCR_MSTP));
  *stbcr = STBCR_AP_VAL;

  cprintf("CPU%d Wakeup!!\n",id);
}

void xv6_smp_reset() {
  scif_putc('Q');
  scif_putc('Q');
  while(1);
}

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting\n", cpu->id);
  cpu->started = 1;
  scheduler();     // start running processes
}

#if 0
pde_t entrypgdir[];  // For entry.S
#endif

// Start the non-boot (AP) processors.
static void
startothers(void)
{
  cprintf("%s\n", __func__);
#if 0
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
#endif
  struct cpu *c;
  char *stack;

#if 0
  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = p2v(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);
#endif

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == cpus)  // We've started already.
      continue;

#if 0
    // Tell entryother.S what stack to use, where to enter, and what 
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    // kalloc can return addresses above 4Mbyte (the machine may have 
    // much more physical memory than 4Mbyte), which aren't mapped by
    // entrypgdir, so we must allocate a stack using enter_alloc(); 
    // this introduces the constraint that xv6 cannot use kalloc until 
    // after these last enter_alloc invocations.
    stack = enter_alloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void**)(code-8) = mpenter;
    *(int**)(code-12) = (void *) v2p(entrypgdir);

    lapicstartap(c->id, v2p(code));
#else
    stack = enter_alloc();
    xv6_smp_stack = stack + KSTACKSIZE;
    c->id = c - cpus;
    slave_boot(smp_start, c->id);
#endif

    // wait for cpu to finish mpmain()
    //while(c->started == 0)
    //  ;
  }
}

#if 0
// Boot page table used in entry.S and entryother.S.
// Page directories (and page tables), must start on a page boundary,
// hence the "__aligned__" attribute.  
// Use PTE_PS in page directory entry to enable 4Mbyte pages.
__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) + PTE_P + PTE_W + PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) + PTE_P + PTE_W + PTE_PS,
};
#endif

//PAGEBREAK!
// Blank page.

