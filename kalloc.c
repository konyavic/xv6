// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

extern unsigned const char *__bss_end;
static char *newend;

// A simple page allocator to get off the ground during entry
char *
enter_alloc(void)
{
  if (newend == 0)
    newend = &__bss_end;

#if 0
  if ((uint) newend >= KERNBASE + 0x400000)
    panic("only first 4Mbyte are mapped during entry");
#endif
  void *p = (void*)PGROUNDUP((uint)newend);
  memset(p, 0, PGSIZE);
  newend = newend + PGSIZE;
  return p;
}

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;

  initlock(&kmem.lock, "kmem");
  if (newend == 0)
    newend = &__bss_end;
  p = (char*)PGROUNDUP((uint)newend);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE)
    kfree(p);
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || (uint)v < 1024*1024 || (uint)v >= PHYSTOP) 
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    memset(r, 0, PGSIZE);
  }
  release(&kmem.lock);
#ifdef DEBUG
  cprintf("%s: 0x%x\n", __func__, r);
#endif
  return (char*)r;
}

