# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save current register context in old
# and then load register context from new.

.macro debug  ! r5 = arg
  mov.l _debug_str, r4
  mov.l _cprintf, r1
  jsr @r1
  nop
.endm

.globl swtch
swtch:
  # swtch(&cpu->scheduler, proc->context);
  # r4 = &cpu->scheduler
  # r5 = proc->context

  /* callee-save */
  ! mov.l r14, @-r15
  ! sts.l pr, @-r15
  ! add -<total sum of local varibles' size>, r15
  ! mov r15, r14

  /* start of store */
  /* system registers */
  mov.l r15,  @-r15
  stc.l sr,   @-r15
  sts.l pr,   @-r15
  sts.l macl, @-r15
  sts.l mach, @-r15
  stc.l gbr,  @-r15
  /* general purpose registers (bank1) */
  stc.l r7_bank,  @-r15
  stc.l r6_bank,  @-r15
  stc.l r5_bank,  @-r15
  stc.l r4_bank,  @-r15
  stc.l r3_bank,  @-r15
  stc.l r2_bank,  @-r15
  stc.l r1_bank,  @-r15
  stc.l r0_bank,  @-r15
  /* control registers */
  stc.l spc,  @-r15
  stc.l ssr,  @-r15
  /* not-banked registers */
  mov.l r14,  @-r15
  mov.l r13,  @-r15
  mov.l r12,  @-r15
  mov.l r11,  @-r15
  mov.l r10,  @-r15
  mov.l r9,   @-r15
  mov.l r8,   @-r15
  /* general purpose registers (bank0) */
  mov.l r7,   @-r15
  mov.l r6,   @-r15
  mov.l r5,   @-r15
  mov.l r4,   @-r15
  mov.l r3,   @-r15
  mov.l r2,   @-r15
  mov.l r1,   @-r15
  mov.l r0,   @-r15
  /* return value */
  mov.l r15, @r4
  /* end of store */

  mov r5, r15
  /* start of load */
  /* general purpose registers (bank0) */
  mov.l @r15+, r0
  mov.l @r15+, r1
  mov.l @r15+, r2
  mov.l @r15+, r3
  mov.l @r15+, r4
  mov.l @r15+, r5
  mov.l @r15+, r6
  mov.l @r15+, r7
  /* not-banked registers */
  mov.l @r15+, r8
  mov.l @r15+, r9
  mov.l @r15+, r10
  mov.l @r15+, r11
  mov.l @r15+, r12
  mov.l @r15+, r13
  mov.l @r15+, r14
  /* control registers */
  ldc.l @r15+, ssr
  ldc.l @r15+, spc
  /* general purpose registers (bank1) */
  ldc.l @r15+, r0_bank
  ldc.l @r15+, r1_bank
  ldc.l @r15+, r2_bank
  ldc.l @r15+, r3_bank
  ldc.l @r15+, r4_bank
  ldc.l @r15+, r5_bank
  ldc.l @r15+, r6_bank
  ldc.l @r15+, r7_bank
  /* system registers */
  ldc.l @r15+, gbr
  lds.l @r15+, mach
  lds.l @r15+, macl
  lds.l @r15+, pr
  ldc.l @r15+, sr
  mov.l @r15+, r15
  /* end of load */

  /* return */
  ! add <total sum of local varibles' size>, r14
  ! mov r14, r15
  ! lds.l @r15+, pr
  ! mov.l @r15+, r14
  rts
  nop
