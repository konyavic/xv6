// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "sh4.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
acquire(struct spinlock *lk)
{
  pushcli(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    panic("acquire");

  // The xchg is atomic.
  // It also serializes, so that reads after acquire are not
  // reordered before it. 
#if 0
  while(xchg(&lk->locked, 1) != 0)
    ;
#else
  unsigned long tmp;

  __asm__ __volatile__ (
      "\n\t1:"
      "\n\tmovli.l  @%1, %0"
      "\n\tcmp/pl   %0"
      "\n\tbt       1b"
      "\n\tmov      #1, %0"
      "\n\tmovco.l  %0, @%1"
      "\n\tbf       1b"
      : "=&z" (tmp) // 'z' means to use r0
      : "r" (&lk->locked)
      : "t", "memory"
      );
#endif

  // Record info about lock acquisition for debugging.
  lk->cpu = cpu;
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // The xchg serializes, so that reads before release are 
  // not reordered after it.  The 1996 PentiumPro manual (Volume 3,
  // 7.2) says reads can be carried out speculatively and in
  // any order, which implies we need to serialize here.
  // But the 2007 Intel 64 Architecture Memory Ordering White
  // Paper says that Intel 64 and IA-32 will not move a load
  // after a store. So lock->locked = 0 would work here.
  // The xchg being asm volatile ensures gcc emits it after
  // the above assignments (and after the critical section).
#if 0
  xchg(&lk->locked, 0);
#else
  // It does not require additional sync in unlock.
  lk->locked = 0;
#endif

  popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
void
getcallerpcs(void *v, uint pcs[])
{
  // XXX: Due to a different frame in SH4,
  // it is left for further works.
  uint *ebp;
  int i;
 
  ebp = (uint*)v - 2;
  for(i = 0; i < 10; i++){
    break;
    if(ebp == 0 || ebp < (uint*)0x100000 || ebp == (uint*)0xffffffff)
      break;
    pcs[i] = ebp[1];     // saved %eip
    ebp = (uint*)ebp[0]; // saved %ebp
  }
  for(; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int
holding(struct spinlock *lock)
{
  return lock->locked && lock->cpu == cpu;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void
pushcli(void)
{
#if 0
  int eflags;
  
  eflags = readeflags();
  cli();
  if(cpu->ncli++ == 0)
    cpu->intena = eflags & FL_IF;
#else
  int sr;
  
  sr = read_sr();
  cli();
  if(cpu->ncli++ == 0)
    cpu->intena = sr & SR_BL_MASK;
#endif
}

void
popcli(void)
{
#if 0
  if(readeflags()&FL_IF)
    panic("popcli - interruptible");
  if(--cpu->ncli < 0)
    panic("popcli");
  if(cpu->ncli == 0 && cpu->intena)
    sti();
#else
  if(!(read_sr() & SR_BL_MASK))
    panic("popcli - interruptible");
  if(--cpu->ncli < 0)
    panic("popcli");
  if(cpu->ncli == 0 && !cpu->intena)
    sti();
#endif
}

