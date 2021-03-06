#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "sh4.h"
#include "proc.h"
#include "spinlock.h"

void do_timer(void);

void timer_init(void)
{
  /* stop count TCNTx (not change other TCNT)*/
#if defined(SH7780) || defined(SH7751)
  TMU.TSTR &= ~TMU_STR0;
#elif defined(RP1)
  TMU.TSTR&=~TMU_STR5;
#endif

  /* set timer constant. */
#if defined(SH7780) || defined(SH7751)
  TMU.TCOR0 = TIMER_RATE;
  TMU.TCNT0 = TIMER_RATE;
#elif defined(RP1)
  TMU.TCOR2 = TIMER_RATE;
  TMU.TCNT2 = TIMER_RATE;
#endif

  /* enable interrupts, counts Pck/3 clock */
#if defined(SH7780) || defined(SH7751)
  TMU.TCR0 &= ~(TCR_UNF_BIT|TCR_UNIE_BIT|TCR_CKEG_MASK|TCR_TPSC_MASK);
#elif defined(RP1)
  TMU.TCR2 &= ~(TCR_UNF_BIT|TCR_UNIE_BIT|TCR_CKEG_MASK|TCR_TPSC_MASK);
#endif
  
  /* clear under flowflag */
#if defined(SH7780) || defined(SH7751)
  TMU.TCR0 |= (TCR_UNIE_BIT | TCR_TPSC_PCK4);
#elif defined(RP1)
  TMU.TCR2 |= (TCR_UNIE_BIT | TCR_TPSC_PCK4);
#endif

#if defined(SH7751)
  IPRA_TMU0(TINTLVL);
  register_handler(TMU0_INTEVT, do_timer);
#elif defined(SH7780)
  IPRA_TMU3(TINTLVL << 1);  /* set interrupt level */
  *IMSKC = IMSK_TMU345;     /* set interrupt mask */
  register_handler(TMU3_INTEVT, do_timer);
#elif defined(RP1)
  IPRA_TMU5(TINTLVL);
  *IMSKC0 = IMSK_TMU5;
  register_handler(TMU5_INTEVT, do_timer);
#endif

  /* start timer */
#if defined(SH7780) || defined(SH7751)
  TMU.TSTR |= TMU_STR0;
#elif defined(RP1)
  TMU.TSTR |= TMU_STR5;
#endif
}

void do_timer(void)
{
#if defined(SH7780) || defined(SH7751)
  TMU.TCR0 &= ~TCR_UNF;
#elif defined(RP1)
  TMU.TCR2 &= ~TCR_UNF;
#endif
  if(cpu->id == 0){
    acquire(&tickslock);
    ticks++;
    wakeup(&ticks);
    release(&tickslock);
  }
  if(proc && proc->killed)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed)
    exit();

  return;
}
