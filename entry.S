#include    "param.h"

.section .start, "ax"

.global start
start:
  /* initialize the cache */
  mov.l   CCR_REG, r0
  mov.l   CCR_INIT, r1
  mov.l   r1, @r0

  /* initialize status registers */
  mov.l   SR_INIT, r1
  ldc     r1, sr

  /* initialize bss */
  mov.l   BSS_END, r1
  mov.l   BSS_START, r2
  mov     #0, r0
1:
  movca.l r0, @r2
  mov.l   r0, @(4,r2)
  mov.l   r0, @(8,r2)
  mov.l   r0, @(12,r2)
  mov.l   r0, @(16,r2)
  mov.l   r0, @(20,r2)
  mov.l   r0, @(24,r2)
  mov.l   r0, @(28,r2)
  add     #32, r2
  cmp/hs  r1, r2
  bf      1b

  /* initialize stack point */
  mov.l   XV6_STACK, r0
  mov     r0, r15

  /* initialize vector base register */
  mov.l   VBR_INIT, r0
  ldc     r0, vbr

  mov.l   XV6_MAIN, r0
  jmp     @r0
  nop

.global smp_start
smp_start:
  /* initialize the cache */
  mov.l   CCR_REG, r0
  mov.l   CCR_INIT, r1
  mov.l   r1, @r0

  /* initialize status registers */
  mov.l   SR_INIT, r1
  ldc     r1, sr

  /* initialize stack point */
  mov.l   XV6_STACK, r0
  mov     r0, r15

  /* initialize vector base register */
  mov.l   VBR_INIT, r0
  ldc     r0, vbr

  mov.l   XV6_SMP_MAIN, r0
  jmp     @r0
  nop

.align 4
CCR_REG:
  .long 0xff00001c
CCR_INIT:
  .long 0x90b
SR_INIT:
  .long 0x70000000
VBR_INIT:
  .long start
BSS_START:
  .long __bss_start
BSS_END:
  .long __bss_end
XV6_MAIN:
  .long main
XV6_SMP_MAIN:
  .long mpenter
XV6_STACK:
  .long xv6_stack+STACK_SIZE

/****************************************************************
*                                                              *
*  general exception handler (VBR + 0x100)                     *
*                                                              *
****************************************************************/
.balign 256, 0, 256
general_exception:
  /*
   * kernel stack -> kernel stack (no change)
   * user stack   -> kernel stack (change)
   */
  stc     ssr, r15
  shll    r15
  shll    r15
  bt/s    excp_no_change  ! SR.MD == 1 ?
  stc     sgr, r15
  /* current stack = r15 = proc->kstack + KSTACKSIZE */
  mov.l r2, @-r15
  mov.l r1, @-r15
  mov.l excp_proc, r1
  mov.l @r1, r1
  mov.l @(8,r1), r1 ! XXX: better way to locate proc->kstack?
  mov.l excp_STACK_SIZE, r2
  add   r2, r1
  mov   r15, r2     ! r2 = old stack
  mov   r1, r15     ! r15 = new stack
  mov.l @r2+, r1
  mov.l @r2+, r2
excp_no_change:
  /* save trapframe */
  sts.l	pr,   @-r15
  sts.l macl, @-r15
  sts.l mach, @-r15
  stc.l gbr,  @-r15
  stc.l sgr,  @-r15
  stc.l ssr,  @-r15
  stc.l spc,  @-r15
  mov.l r14,  @-r15
  mov.l r13,  @-r15
  mov.l r12,  @-r15
  mov.l r11,  @-r15
  mov.l r10,  @-r15
  mov.l r9,   @-r15
  mov.l r8,   @-r15
  mov.l r7,   @-r15
  mov.l r6,   @-r15
  mov.l r5,   @-r15
  mov.l r4,   @-r15
  mov.l r3,   @-r15
  mov.l r2,   @-r15
  mov.l r1,   @-r15
  mov.l r0,   @-r15
  /* save EXPEVT */
  mov.l excp_EXPEVT, r4
  mov.l @r4, r4

#if 0
  /* enable intr */
  stc   sr, r1
  mov.l excp_SR_BL_ENABLE, r2
  and   r1, r2
  ldc   r2, sr
#endif

  /* call the service routine */
  mov.l excp_vectors, r1
  shlr2 r4
  shlr  r4
  add   r4, r1    ! r1 = vectors + EXPEVT >> 3
  mov.l @r1, r1
  jsr   @r1
  nop

#if 0
  /* disable intr */
  stc   sr, r1
  mov.l excp_SR_BL_DISABLE, r2
  or   r1, r2
  ldc   r2, sr
#endif

  /* restore trapframe */
.global trapret
trapret:
#if 0
    stc r0_bank, r5
    mov.l excp_debug_str, r4
    mov.l excp_cprintf, r1
    jsr @r1
    nop
#endif
  mov.l @r15+, r0
  mov.l @r15+, r1
  mov.l @r15+, r2
  mov.l @r15+, r3
  mov.l @r15+, r4
  mov.l @r15+, r5
  mov.l @r15+, r6
  mov.l @r15+, r7
  mov.l @r15+, r8
  mov.l @r15+, r9
  mov.l @r15+, r10
  mov.l @r15+, r11
  mov.l @r15+, r12
  mov.l @r15+, r13
  mov.l @r15+, r14
  ldc.l @r15+, spc
  ldc.l @r15+, ssr
  ! ldc.l @r15+, sgr  ! forbidden
  add   #4, r15
  ldc.l @r15+, gbr
  lds.l @r15+, mach
  lds.l @r15+, macl
  lds.l @r15+, pr
  /* return */
  ! stc sgr, r15
  add #-20, r15
  mov.l @r15, r15
  rte
  nop

#ifdef DEBUG
    mov r15, r5
    mov.l excp_debug_str, r4
    mov.l excp_cprintf, r1
    jsr @r1
    nop
debug_loop_excp:
    bra debug_loop_excp
    nop
#endif

.align 4
excp_EXPEVT: 
  .long 0xff000024
excp_cprintf:
  .long cprintf
excp_debug_str:
  .long debug_str
excp_SR_MD:
  .long 0x40000000
excp_SR_BL_ENABLE:
  .long 0xefffffff
excp_SR_BL_DISABLE:
  .long 0x10000000
excp_proc:
  .long proc
excp_vectors:
  .long vectors
excp_STACK_SIZE:
  .long KSTACKSIZE

/****************************************************************
 *                                                              *
 *  tlb exception handler (VBR + 0x400)                         *
 *                                                              *
 ****************************************************************/
.balign 1024, 0, 1024
tlb_exception:
  /*
   * user stack   -> kernel stack
   */
  /* current stack = r15 = proc->kstack + KSTACKSIZE */
  mov.l tlb_tmp_stack, r15
  mov.l r2, @-r15
  mov.l r1, @-r15
  mov.l tlb_proc, r1
  mov.l @r1, r1
  mov.l @(8,r1), r1 ! XXX: better way to locate proc->kstack?
  mov.l tlb_STACK_SIZE, r2
  add   r2, r1
  mov   r15, r2     ! r2 = old stack
  mov   r1, r15     ! r15 = new stack
  mov.l @r2+, r1
  mov.l @r2+, r2
  /* save trapframe */
  sts.l	pr,   @-r15
  sts.l macl, @-r15
  sts.l mach, @-r15
  stc.l gbr,  @-r15
  stc.l sgr,  @-r15
  stc.l ssr,  @-r15
  stc.l spc,  @-r15
  mov.l r14,  @-r15
  mov.l r13,  @-r15
  mov.l r12,  @-r15
  mov.l r11,  @-r15
  mov.l r10,  @-r15
  mov.l r9,   @-r15
  mov.l r8,   @-r15
  mov.l r7,   @-r15
  mov.l r6,   @-r15
  mov.l r5,   @-r15
  mov.l r4,   @-r15
  mov.l r3,   @-r15
  mov.l r2,   @-r15
  mov.l r1,   @-r15
  mov.l r0,   @-r15
  /* save EXPEVT */
  mov.l tlb_EXPEVT, r4
  mov.l @r4, r4
  /* XXX: enable intr */

  /* call the service routine */
  mov.l tlb_vectors, r1
  shlr2 r4
  shlr  r4
  add   r4, r1  ! r1 = vectors + EXPEVT >> 3
  mov.l @r1, r1
  jsr   @r1
  nop

  /* restore trapframe */
  mov.l @r15+, r0
  mov.l @r15+, r1
  mov.l @r15+, r2
  mov.l @r15+, r3
  mov.l @r15+, r4
  mov.l @r15+, r5
  mov.l @r15+, r6
  mov.l @r15+, r7
  mov.l @r15+, r8
  mov.l @r15+, r9
  mov.l @r15+, r10
  mov.l @r15+, r11
  mov.l @r15+, r12
  mov.l @r15+, r13
  mov.l @r15+, r14
  ldc.l @r15+, spc
  ldc.l @r15+, ssr
  ! ldc.l @r15+, sgr  ! forbidden
  add   #4, r15
  ldc.l @r15+, gbr
  lds.l @r15+, mach
  lds.l @r15+, macl
  lds.l @r15+, pr
  /* return */
  ! stc sgr, r15
  add #-20, r15
  mov.l @r15, r15
  rte
  nop

#ifdef DEBUG
    mov r15, r5
    mov.l tlb_debug_str, r4
    mov.l tlb_cprintf, r1
    jsr @r1
    nop
debug_loop_tlb:
    bra debug_loop_tlb
    nop
#endif

.align 4
tlb_EXPEVT: 
  .long 0xff000024
tlb_cprintf:
  .long cprintf
tlb_debug_str:
  .long debug_str
tlb_SR_MD:
  .long 0x40000000
tlb_proc:
  .long proc
tlb_vectors:
  .long vectors
tlb_STACK_SIZE:
  .long KSTACKSIZE
tlb_tmp_stack:
  .long xv6_stack + STACK_SIZE


/****************************************************************
 *                                                              *
 *  interrupt handler (VBR + 0x600)                             *
 *                                                              *
 ****************************************************************/
.balign 512, 0, 512
intr_entry:
  /*
   * kernel stack -> kernel stack (no change)
   * user stack   -> kernel stack (change)
   */
  stc     ssr, r15
  shll    r15
  shll    r15
  bt/s    intr_no_change  ! SR.MD == 1 ?
  stc     sgr, r15
  /* current stack = r15 = proc->kstack + KSTACKSIZE */
  mov.l r2, @-r15
  mov.l r1, @-r15
  mov.l intr_proc, r1
  mov.l @r1, r1
  mov.l @(8,r1), r1 ! XXX: better way to locate proc->kstack?
  mov.l intr_STACK_SIZE, r2
  add   r2, r1
  mov   r15, r2     ! r2 = old stack
  mov   r1, r15     ! r15 = new stack
  mov.l @r2+, r1
  mov.l @r2+, r2
intr_no_change:
  /* save trapframe */
  sts.l	pr,   @-r15
  sts.l macl, @-r15
  sts.l mach, @-r15
  stc.l gbr,  @-r15
  stc.l sgr,  @-r15
  stc.l ssr,  @-r15
  stc.l spc,  @-r15
  mov.l r14,  @-r15
  mov.l r13,  @-r15
  mov.l r12,  @-r15
  mov.l r11,  @-r15
  mov.l r10,  @-r15
  mov.l r9,   @-r15
  mov.l r8,   @-r15
  mov.l r7,   @-r15
  mov.l r6,   @-r15
  mov.l r5,   @-r15
  mov.l r4,   @-r15
  mov.l r3,   @-r15
  mov.l r2,   @-r15
  mov.l r1,   @-r15
  mov.l r0,   @-r15
  /* save EXPEVT */
  mov.l intr_INTEVT, r4
  mov.l @r4, r4
  /* XXX: enbale intr */
  
  /* call the service routine */
  mov.l intr_vectors, r1
  shlr2 r4
  shlr  r4
  add   r4, r1    ! r1 = vectors + INTEVT >> 3
  mov.l @r1, r1
  jsr   @r1
  nop

  /* restore trapframe */
  mov.l @r15+, r0
  mov.l @r15+, r1
  mov.l @r15+, r2
  mov.l @r15+, r3
  mov.l @r15+, r4
  mov.l @r15+, r5
  mov.l @r15+, r6
  mov.l @r15+, r7
  mov.l @r15+, r8
  mov.l @r15+, r9
  mov.l @r15+, r10
  mov.l @r15+, r11
  mov.l @r15+, r12
  mov.l @r15+, r13
  mov.l @r15+, r14
  ldc.l @r15+, spc
  ldc.l @r15+, ssr
  ! ldc.l @r15+, sgr  ! forbidden
  add   #4, r15
  ldc.l @r15+, gbr
  lds.l @r15+, mach
  lds.l @r15+, macl
  lds.l @r15+, pr
  /* return */
  ! stc sgr, r15
  add #-20, r15
  mov.l @r15, r15
  rte
  nop

.align 4
intr_INTEVT: 
  .long 0xff000028
intr_cprintf:
  .long cprintf
intr_debug_str:
  .long debug_str
intr_SR_MD:
  .long 0x40000000
intr_proc:
  .long proc
intr_vectors :
  .long vectors
intr_STACK_SIZE:
  .long KSTACKSIZE
