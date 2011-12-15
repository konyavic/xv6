#include "param.h"
#include "types.h"
#include "defs.h"
#include "sh4.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

#define USERTOP  0x80000000

static pde_t *kpgdir;  // for use in scheduler()
extern struct cpu *bcpu;

// Set up CPU's kernel segment descriptors.
// Run once at boot time on each CPU.
void
ksegment(void)
{
  // MP is not supported
  // SH4A has no segment
  bcpu = &cpus[0];
  cpu = bcpu;
  proc = 0;
}

// Return the address of the PTE in page table pgdir
// that corresponds to linear address va.  If create!=0,
// create any required page table pages.
static pde_t *
walkpgdir(pde_t *pgdir, const void *va, int create)
{
  uint r;
  pde_t *pde;
  pde_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTEL_V){
    pgtab = (pde_t*) PTE_ADDR(*pde);
  } else if(!create || !(r = (uint) kalloc()))
    return 0;
  else {
    pgtab = (pde_t*) r;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table 
    // entries, if necessary.
    *pde = PADDR(r) | PTEL_DEFAULT;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for linear addresses starting at la that refer to
// physical addresses starting at pa. la and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *la, uint size, uint pa, int perm)
{
  char *a = PGROUNDDOWN(la);
  char *last = PGROUNDDOWN(la + size - 1);

  while(1){
    pde_t *pte = walkpgdir(pgdir, a, 1);
    if(pte == 0)
      return 0;
    if(*pte & PTEL_V)
      panic("remap");
    *pte = pa | perm | PTEL_V;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 1;
}


// The mappings from logical to linear are one to one (i.e.,
// segmentation doesn't do anything).
// There is one page table per process, plus one that's used
// when a CPU is not running any process (kpgdir).
// A user process uses the same page table as the kernel; the
// page protection bits prevent it from using anything other
// than its memory.
// 
// setupkvm() and exec() set up every page table like this:
//   0..640K          : user memory (text, data, stack, heap)
//   640K..1M         : mapped direct (for IO space)
//   1M..end          : mapped direct (for the kernel's text and data)
//   end..PHYSTOP     : mapped direct (kernel heap and user pages)
//   0xfe000000..0    : mapped direct (devices such as ioapic)
//
// The kernel allocates memory for its heap and for user memory
// between kernend and the end of physical memory (PHYSTOP).
// The virtual address space of each user program includes the kernel
// (which is inaccessible in user mode).  The user program addresses
// range from 0 till 640KB (USERTOP), which where the I/O hole starts
// (both in physical memory and in the kernel's virtual address
// space).

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void
kvmalloc(void)
{
  kpgdir = setupkvm();
}

// Set up kernel part of a page table.
pde_t*
setupkvm(void)
{
  pde_t *pgdir;

  // Allocate page directory
  if(!(pgdir = (pde_t *) kalloc()))
    return 0;
#ifdef DEBUG
  cprintf("%s: pgdir=0x%x\n", __func__, pgdir);
#endif
  memset(pgdir, 0, PGSIZE);
  // SH4A do not have to map the kernel space again,
  // it can access P1 directly
  //if(!mappages(pgdir, (void *)0x8c800000, PHYSTOP-0x8c800000, 0x0c800000, PTE_W))
  //  return 0;
  return pgdir;
}

// Turn on paging.
void
vmenable(void)
{
  //uint cr0;
  switchkvm();
  enable_mmu();
   // load kpgdir into cr3
  //cr0 = rcr0();
  //cr0 |= CR0_PG;
  //lcr0(cr0);
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm()
{
#ifdef DEBUG
  cprintf("%s:\n", __func__);
#endif
  // Do nothing because kernel space do not have to be mapped in SH4A
}

// Switch h/w page table and TSS registers to point to process p.
void
switchuvm(struct proc *p)
{
#ifdef DEBUG
  cprintf("%s: start\n", __func__);
#endif
  
  pushcli();

  if(p->pgdir == 0)
    panic("switchuvm: no pgdir\n");

  //disable_mmu();
  clear_tlb();

  // load TLB for current process
  // XXX: should be done in TLB miss
  char *va;
  pde_t *pte;
  int i;
  for (
      va = 0, i = 0; 
      ((pte = (pde_t *) walkpgdir(proc->pgdir, va, 0)) != 0) && *pte != 0
      ; va += PGSIZE, i = (i+1)%64
      ) {
    set_urc(i);
    tlb_register(va);
  }
  
  //enable_mmu();
  
  popcli();

#ifdef DEBUG
  cprintf("%s: end\n", __func__);
#endif
}

// Return the physical address that a given user address
// maps to.  The result is also a kernel logical address,
// since the kernel maps the physical memory allocated to user
// processes directly.
char*
uva2ka(pde_t *pgdir, char *uva)
{    
  pde_t *pte = walkpgdir(pgdir, uva, 0);
  if(pte == 0) return 0;
  uint pa = PTE_ADDR(*pte);
  return (char *)pa;
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem = kalloc();
#ifdef DEBUG
  cprintf("%s: mem=0x%x\n", __func__, mem);
#endif
  if (sz >= PGSIZE)
    panic("inituvm: more than a page");
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, PADDR(mem), PTEL_DEFAULT);
  memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pde_t *pte;

  if((uint)addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned\n");
  for(i = 0; i < sz; i += PGSIZE){
    if(!(pte = walkpgdir(pgdir, addr+i, 0)))
      panic("loaduvm: address should exist\n");
    pa = PTE_ADDR(*pte);
    if(sz - i < PGSIZE) n = sz - i;
    else n = PGSIZE;
    if(readi(ip, (char *)pa, offset+i, n) != n)
      return 0;
  }
  return 1;
}

// Allocate memory to the process to bring its size from oldsz to
// newsz. Allocates physical memory and page table entries. oldsz and
// newsz need not be page-aligned, nor does newsz have to be larger
// than oldsz.  Returns the new process size or 0 on error.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  if(newsz > USERTOP)
    return 0;
  char *a = (char *)PGROUNDUP(oldsz);
  char *last = PGROUNDDOWN(newsz - 1);
  for (; a <= last; a += PGSIZE){
    char *mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    mappages(pgdir, a, PGSIZE, PADDR(mem), PTEL_DEFAULT);
  }
  return newsz > oldsz ? newsz : oldsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *a = (char *)PGROUNDUP(newsz);
  char *last = PGROUNDDOWN(oldsz - 1);
  for(; a <= last; a += PGSIZE){
    pde_t *pte = walkpgdir(pgdir, a, 0);
    if(pte && (*pte & PTEL_V) != 0){
      uint pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      kfree((void *) pa);
      *pte = 0;
    }
  }
  return newsz < oldsz ? newsz : oldsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
#ifdef DEBUG
  cprintf("%s:\n", __func__);
#endif
  uint i;
  if(!pgdir)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, USERTOP, 0);  // XXX: heavy
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTEL_V) {
#ifdef DEBUG
      cprintf("%s: free page addr=0x%x\n", __func__, PTE_ADDR(pgdir[i]));
#endif
      kfree((void *) PTE_ADDR(pgdir[i]));
    }
  }
#ifdef DEBUG
  cprintf("%s: free pgdir=0x%x\n", __func__, pgdir);
#endif
  kfree((void *) pgdir);
}



// Given a parent process's page table, create a copy
// of it for a child.
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d = setupkvm();
  pde_t *pte;
  uint pa, i;
  char *mem;

  if(!d) return 0;
  for(i = 0; i < sz; i += PGSIZE){
    if(!(pte = walkpgdir(pgdir, (void *)i, 0)))
      panic("copyuvm: pte should exist\n");
    if(!(*pte & PTEL_V))
      panic("copyuvm: page not present\n");
    pa = PTE_ADDR(*pte);
    if(!(mem = kalloc()))
      goto bad;
    memmove(mem, (char *)pa, PGSIZE);
    if(!mappages(d, (void *)i, PGSIZE, PADDR(mem), PTEL_DEFAULT))
      goto bad;
  }
  return d;

bad:
  freevm(d);
  return 0;
}

void tlb_register(char *va) 
{
  pde_t *pte = walkpgdir(proc->pgdir, va, 0);
  uint pa = PTE_ADDR(*pte);
  uint perm = PTE_PERM(*pte);
  set_pteh(PTE_ADDR(va));
  set_ptel(pa|perm);
#ifdef DEBUG
  cprintf("%s: va=0x%x PTEH=0x%x PTEL=0x%x\n", __func__, 
      va, *(uint *)PTEH, *(uint *)PTEL);
#endif
  ldtlb();
}

void do_tlb_miss()
{
  // XXX: current irq lock in acquire() forbid TLB miss 
  // and causes reset

  //char *va = *(char **)TEA;
  //tlb_register(va);
  return;
}

void do_tlb_violation()
{
  cprintf("pid %d %s: access violation -- killed\n", proc->pid, proc->name);
  proc->killed = 1;
  exit();
}

char dump_head[] = "        ";
void dump_pde(pde_t *pde, int also_dump_mem, int level) 
{
  char *head = dump_head + sizeof(dump_head) - level - 1;
  cprintf("%s--- %s start ---\n", head, __func__);
  int i;
  int skip = 0;
  for (i = 0; i < PGSIZE/4; i += 8) {
    if (i != 0 
        && pde[i] == pde[i-8]
        && pde[i+1] == pde[i-7]
        && pde[i+2] == pde[i-6]
        && pde[i+3] == pde[i-5]
        && pde[i+4] == pde[i-4]
        && pde[i+5] == pde[i-3]
        && pde[i+6] == pde[i-2]
        && pde[i+7] == pde[i-1]
        ) {
      if (!skip) {
        cprintf("%s *\n", head);
        skip = 1;
      }
      continue;
    }
    skip = 0;
    cprintf(
        "%s[0x%x] "
        "0x%x, "
        "0x%x, "
        "0x%x, "
        "0x%x, "
        "0x%x, "
        "0x%x, "
        "0x%x, "
        "0x%x\n",
        head,
        pde+i,
        pde[i],
        pde[i+1],
        pde[i+2],
        pde[i+3],
        pde[i+4],
        pde[i+5],
        pde[i+6],
        pde[i+7]
        );
    if (!also_dump_mem)
      continue;

    int j;
    for (j = 0; j < 8; ++j) {
      if (pde[i+j] != 0) {
        dump_mem((char *)PTE_ADDR(pde[i+j]), PGSIZE, level + 2);
      }
    }
  }
  cprintf("%s--- %s end ---\n", head, __func__);
}

void dump_pgd(pde_t *pgd, int level) 
{
  char *head = dump_head + sizeof(dump_head) - level - 1;
  cprintf("%s--- %s start ---\n", head, __func__);
  int i;
  for (i = 0; i < PGSIZE/4; i += 8) {
    if (pgd[i] != 0) {
      cprintf("%spte=0x%x\n", head, pgd[i]);
      dump_pde((pde_t *)PTE_ADDR(pgd[i]), 1, level+2);
    }
  }
  cprintf("%s--- %s end ---\n", head, __func__);
}

void dump_mem(char *addr, int size, int level) 
{
  char *head = dump_head + sizeof(dump_head) - level - 1;
  cprintf("%s--- %s start ---\n", head, __func__);
  int i;
  int skip = 0;
  for (i = 0; i < PGSIZE; i += 16) {
    if (i != 0 
        && addr[i] == addr[i-16]
        && addr[i+1] == addr[i-15]
        && addr[i+2] == addr[i-14]
        && addr[i+3] == addr[i-13]
        && addr[i+4] == addr[i-12]
        && addr[i+5] == addr[i-11]
        && addr[i+6] == addr[i-10]
        && addr[i+7] == addr[i-9]
        && addr[i+8] == addr[i-8]
        && addr[i+9] == addr[i-7]
        && addr[i+10] == addr[i-6]
        && addr[i+11] == addr[i-5]
        && addr[i+12] == addr[i-4]
        && addr[i+13] == addr[i-3]
        && addr[i+14] == addr[i-2]
        && addr[i+15] == addr[i-1]
        ) {
      if (!skip) {
        cprintf("%s *\n", head);
        skip = 1;
      }
      continue;
    }
    skip = 0;
    cprintf(
        "%s[0x%x] "
        "%x %x %x %x %x %x %x %x "
        "%x %x %x %x %x %x %x %x\n",
        head,
        addr+i,
        addr[i],
        addr[i+1],
        addr[i+2],
        addr[i+3],
        addr[i+4],
        addr[i+5],
        addr[i+6],
        addr[i+7],
        addr[i+8],
        addr[i+9],
        addr[i+10],
        addr[i+11],
        addr[i+12],
        addr[i+13],
        addr[i+14],
        addr[i+15]
        );
  }
  cprintf("%s--- %s end ---\n", head, __func__);
}
