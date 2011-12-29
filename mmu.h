/* registers */
#define PTEH 0xff000000  // page-table entry high
#define PTEL 0xff000004  // page-table entry low
#define TTB  0xff000008  // transfer-table base
#define TEA  0xff00000c  // TLB exception address
#define MMUCR 0xff000010 // MMU control register
#define PASCR 0xff000070 // phisical-address-space control register
#define IRMCR 0xff000078 // instruction re-fetch control register
#define PTEA  0xff000034 //Page table entry assistance register 

// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/ 

// page directory index
#define PDX(va)         (((uint)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va)         (((uint)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PGSHIFT         12      // log2(PGSIZE)
#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

// for SH4 and SH4A
// PTEH
// +-----------+--+----------+
// | VPN 31:10 |xx| ASID 7:0 |
// +-----------+--+----------+
//
// PTEL
// +---+-----------+-+-+--+--+--+--+-+-+--+--+
// |xxx| PPN 28:10 |x|V|SZ|PR|PR|SZ|C|D|SH|WT|
// +---+-----------+-+-+--+--+--+--+-+-+--+--+
//
// pte
// +-----------+---+-+--+--+--+--+-+-+--+--+
// | PPN 31:12 |xxx|V|SZ|PR|PR|SZ|C|D|SH|WT|
// +-----------+---+-+--+--+--+--+-+-+--+--+

// Page table/directory entry flags.
#define	PTEL_WT         0x001 /* WT-bit on SH-4, 0 on SH-3 */
#define PTEL_HW_SHARED  0x002 /* SH-bit  : shared among processes */
#define PTEL_DIRTY      0x004 /* D-bit   : page changed */
#define PTEL_CACHABLE   0x008 /* C-bit   : cachable */
#define PTEL_SZ0        0x010 /* SZ0-bit : Size of page */
#define PTEL_RW         0x020 /* PR0-bit : write access allowed */
#define PTEL_USER       0x040 /* PR1-bit : user space access allowed */
#define PTEL_SZ1        0x080 /* SZ1-bit : Size of page (on SH-4) */
#define PTEL_V          0x100 /* V-bit   : page is valid */
#define PTEL_DEFAULT    ( PTEL_WT | PTEL_SZ0 | PTEL_RW | PTEL_USER | PTEL_V | PTEL_DIRTY )
// XXX: why causing miltiple hit without dirty?

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)
#define PTE_PERM(pte)	((uint)(pte) & 0x1FF)

#ifndef __ASSEMBLER__
typedef uint pte_t;

inline static void enable_mmu()
{
  uint * mmucr = (uint *)MMUCR;
  *mmucr |= 0x00000001;
  return;
}

inline static void disable_mmu()
{
  uint * mmucr = (uint *)MMUCR;
  *mmucr &= 0xfffffffe;
  return;
}

inline static void clear_tlb()
{
  uint * mmucr = (uint *)MMUCR;
  *mmucr |= 0x00000004;
  return;
}

inline static void set_urb(uint val)
{
  uint * mmucr = (uint *)MMUCR;
  if(val >= 64)
  {
    val = 63;
  }
  val <<= 18;
  val &= 0x00fc0000;
  *mmucr &= 0xff03ffff;
  *mmucr |= val;
  return;

}

inline static void set_urc(uint val)
{
  uint * mmucr = (uint *)MMUCR;
  if(val >= 64)
  {
    val = 63;
  }
  val <<= 10;
  val &= 0x0000fc00;
  *mmucr &= 0xffff03ff;
  *mmucr |= val;
  return;
}

inline static void set_pteh(uint val)
{
  uint * pteh = (uint *)PTEH;
  *pteh = (val & 0xffffffff);
  return;
}

inline static void set_ptea(uint val)
{
  uint * ptea = (uint *)PTEA;
  *ptea = (val & 0xffffffff);
  return;
}

inline static void set_ptel(uint val)
{
  uint * ptel = (uint *)PTEL;
  *ptel = (val & 0xffffffff);
  return;
}

inline static void ldtlb()
{
  asm volatile("ldtlb\n\t");
  return;
}

inline static void set_ttb(uint val)
{
  uint * ttb = (uint *)TTB;
  * ttb = val;
  return;
}

#endif
