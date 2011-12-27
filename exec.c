#include "types.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "sh4.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
#ifdef DEBUG
  cprintf("%s: path=%s\n", __func__, path);
  char **tmp;
  for(tmp=argv; *tmp != 0; ++tmp) {
    cprintf("%s: argv[%d]=%s\n", __func__, tmp-argv, *tmp);
  }
#endif
#ifdef DEBUGxxx
  cprintf("%s: oldpgdir=0x%x\n", __func__, proc->pgdir);
  dump_pde(proc->pgdir, 0, 2);
#endif
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  if((ip = namei(path)) == 0)
    return -1;
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

#ifdef DEBUG
  cprintf("%s: start of loading\n", __func__);
#endif
  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.va + ph.memsz)) == 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.va, ip, ph.offset, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  ip = 0;
#ifdef DEBUG
  cprintf("%s: end of loading\n", __func__);
#endif

  // Allocate a one-page stack at the next page boundary
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + PGSIZE)) == 0)
    goto bad;
#ifdef DEBUGxxx
  cprintf("%s: allocated user stack\n", __func__);
  dump_pgd(pgdir, 2);
#endif

  // Push argument strings, prepare rest of stack in ustack.
  sp = sz;
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp -= strlen(argv[argc]) + 1;
    sp &= ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[argc] = sp;
  }
  ustack[argc] = 0;

#if 0
  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;
#else
  sp -= (argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (argc+1)*4) < 0)
    goto bad;

  uint fakeret = 0xffffffff;
  asm volatile("ldc %0, r4_bank" :: "r"(argc));
  asm volatile("ldc %0, r5_bank" :: "r"(sp));
  asm volatile("lds %0, pr" :: "r"(fakeret));  // fake return pc
#endif

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  // Commit to the user image.
  oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz;
#if 0
  proc->tf->eip = elf.entry;  // main
  proc->tf->esp = sp;
  switchuvm(proc);
#else
  proc->tf->spc = elf.entry;
  proc->tf->sgr = sp;
#ifdef DEBUG
  cprintf("%s: spc=0x%x, sgr=0x%x\n", 
      __func__, proc->tf->spc, proc->tf->sgr);
#endif
  switchuvm(proc);
#endif


  freevm(oldpgdir);

#ifdef DEBUG
  cprintf("%s: finish\n", __func__);
#endif
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip)
    iunlockput(ip);
  return -1;
}
