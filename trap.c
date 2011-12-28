#include "types.h"
#include "defs.h"
#include "param.h"
//#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "sh4.h"
#include "spinlock.h"

handler_t vectors[VECTOR_SIZE];
struct spinlock tickslock;
uint ticks;

void trap(uint evt);

void
tvinit(void)
{
  initlock(&tickslock, "time");
  int i;
  for (i = 0; i < NELEM(vectors); ++i) {
    vectors[i] = (handler_t) trap;
  }

  // machine-independant events
  register_handler(TLB_R_EXPEVT, do_tlb_miss);
  register_handler(TLB_W_EXPEVT, do_tlb_miss);
  register_handler(PERM_R_EXPEVT, do_tlb_violation);
  register_handler(PERM_W_EXPEVT, do_tlb_violation);
  register_handler(ADDR_R_EXPEVT, do_tlb_violation);
  register_handler(ADDR_W_EXPEVT, do_tlb_violation);
  register_handler(SYSCALL_EXPEVT, syscall);
}

void register_handler(uint evt, handler_t handler) 
{
  uint i = evt >> 5;
  if (vectors[i] != (handler_t) trap) {
    cprintf("warning: re-assigning handler for EVT=0x%x\n", evt);
    cprintf("address of original handler: 0x%x\n", vectors[i]);
  }
  vectors[i] = handler;
}

void
trap(uint evt)
{
  cprintf("unexpected trap EVT=0x%x\n", evt << 3);
  while(1);
  return;
}
