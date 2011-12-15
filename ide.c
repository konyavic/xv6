// Simple PIO-based (non-DMA) IDE driver code.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sh4.h"
#include "spinlock.h"
#include "buf.h"

extern char _binary_fs_img_start[];
extern char _binary_fs_img_end[];
extern char _binary_fs_img_size[];

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;

static void idestart(struct buf*);

void
ideinit(void)
{
  initlock(&idelock, "ide");
}

// Start the request for b.  Caller must hold idelock.
static void
idestart(struct buf *b)
{
  char *fstart = _binary_fs_img_start;
  
  if(b == 0)
    panic("idestart");

  char *fdata = fstart + (b->sector * 512);

  if(b->flags & B_DIRTY){
    memmove(fdata, b->data, 512);
  } else {
    memmove(b->data, fdata, 512);
  }
}


// Sync buf with disk. 
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void
iderw(struct buf *b)
{
  if(!(b->flags & B_BUSY))
    panic("iderw: buf not busy");
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");

  acquire(&idelock);

  idestart(b);
  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;

  // Wait for request to finish.
  // Assuming will not sleep too long: ignore proc->killed.
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID) {
    sleep(b, &idelock);
  }

  release(&idelock);
}
