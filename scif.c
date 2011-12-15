#include "types.h"
#include "defs.h"
#include "param.h"
#include "sh4.h"

void do_scif_read(void);

void scif_init(void)
{
  SCIF.SCSCR = 0; /* disable everything */

  SCIF.SCFCR = SCFCR_RFRST | SCFCR_TFRST |    /* clear FIFO */
    SCFCR_TTRG1 | SCFCR_RTRG1; /* set FIFO trigger */

  SCIF.SCSMR = 0; /* 8 bits data, No parity, 1 stop bit */

  SCIF.SCBRR = SCIF_BPS_VALUE; /* set baudrate to 115200bps */

  SCIF.SCFCR = 0;

  SCIF.SCSCR = SCSCR_RIE | SCSCR_TE | SCSCR_RE; /* enable transmit */

  IPRC_SCIF(TINTLVL);
#if defined(SH7780) || defined(SH7751)
  *IMSKC = INTR_CLEAR_SCIF;
#elif defined(RP1)
  *IMSKC2 = IMSK_SCIF;
#endif

  register_handler(SCIF_RXI_INTEVT, (handler_t) do_scif_read);
}

int scif_putc(int c)
{
  while(!(SCIF.SCFSR & SCFSR_TDFE));
  SCIF.SCFTDR = c;
  SCIF.SCFSR &= ~(SCFSR_TDFE|SCFSR_TEND);
  return c;
}

int scif_get(void)
{
  unsigned char c;
  unsigned int s;

#if defined(SH7751)
  s = SCIF.SCFDR & 0xFF;
#elif defined(SH7780)
  s = SCIF.SCFSR & SCFSR_RDF;
#elif defined(RP1)
  s = SCIF.SCFSR & SCFSR_RDF;
#endif
  if (!s)
    return -1;

  c = SCIF.SCFRDR;
  SCIF.SCFSR &= ~(SCFSR_RDF);
  return c;
}

/* interrupt handler */
void do_scif_read(void)
{
  consoleintr(scif_get);
  return;
}

int putc(int c)
{
  scif_putc(c);
  if(c == '\n') {
    scif_putc('\r');
  }
  return c;
}
