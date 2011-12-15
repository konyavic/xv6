# Initial process execs /init.

#include "syscall.h"

# exec(init, argv)
.global start
start:
	mov.l 1f, r4
	mov.l 2f, r5
	TRAPA #SYS_exec
  nop

# should not reach here
3:
  bra 3b
  nop

# char init[] = "/init\0";
1: 
  .long init
init:
  .string "/init\0"

.align 4
# char *argv[] = { init, 0 };
2: 
  .long argv
argv:
  .long init
  .long 0
