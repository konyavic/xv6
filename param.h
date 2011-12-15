#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NBUF         10  // size of disk block cache
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk

#define STACK_SIZE  0x1000

#define TINTLVL        0xd
#define TIMER_RATE	(12500000UL)  /* use PCK/4 edge.  12.5Mhz 1tick = 0.08us */
