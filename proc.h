
// Per-CPU state
struct cpu {
  uchar id;                    // Local APIC ID; index into cpus[] below
  struct context *scheduler;   // Switch here to enter scheduler
  volatile uint booted;        // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?

  // Cpu-local storage variables; see below
  struct cpu *cpu;
  struct proc *proc;
};

struct cpu cpus[1];
struct cpu *bcpu;
char *kstack;

// Per-CPU variables, holding pointers to the
// current cpu and to the current process.
// The asm suffix tells gcc to use "%gs:0" to refer to cpu
// and "%gs:4" to refer to proc.  ksegment sets up the
// %gs segment register so that %gs refers to the memory
// holding those two variables in the local cpu's struct cpu.
// This is similar to how thread-local variables are implemented
// in thread libraries such as Linux pthreads.
struct cpu *cpu;       // This cpu.
struct proc *proc;     // Current proc on this cpu.

// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  /* general purpose registers (bank0) */
  uint r0;
  uint r1;
  uint r2;
  uint r3;
  uint r4;
  uint r5;
  uint r6;
  uint r7;
  /* not-banked registers */
  uint r8;
  uint r9;
  uint r10;
  uint r11;
  uint r12;
  uint r13;
  uint r14;
  /* control registers */
  uint ssr;
  uint spc;
  //uint sgr;
  //_reg_vt dbr;
  /* general purpose registers (bank1) */
  uint r0_bank;
  uint r1_bank;
  uint r2_bank;
  uint r3_bank;
  uint r4_bank;
  uint r5_bank;
  uint r6_bank;
  uint r7_bank;
  /* system registers */
  uint gbr;
  uint mach;
  uint macl;
  uint pr;
  uint sr;
  uint r15;
};

struct trapframe {
  /* general purpose registers */
  uint r0;
  uint r1;
  uint r2;
  uint r3;
  uint r4;
  uint r5;
  uint r6;
  uint r7;
  /* not-banked registers */
  uint r8;
  uint r9;
  uint r10;
  uint r11;
  uint r12;
  uint r13;
  uint r14;
  /* control registers */
  uint spc;
  uint ssr;
  uint sgr;
  uint gbr;
  /* system registers */
  uint mach;
  uint macl;
  uint pr;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc{
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Linear address of proc's pgdir
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  volatile int pid;            // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // Switch here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
