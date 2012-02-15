
/*
 *  TOPPERS/JSP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Just Standard Profile Kernel
 * 
 *  Copyright (C) 2000 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 * 
 *  上記著作権者は，Free Software Foundation によって公表されている 
 *  GNU General Public License の Version 2 に記述されている条件か，以
 *  下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェア（本ソフトウェ
 *  アを改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを再利用可能なバイナリコード（リロケータブルオブ
 *      ジェクトファイルやライブラリなど）の形で利用する場合には，利用
 *      に伴うドキュメント（利用者マニュアルなど）に，上記の著作権表示，
 *      この利用条件および下記の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを再利用不可能なバイナリコードの形または機器に組
 *      み込んだ形で利用する場合には，次のいずれかの条件を満たすこと．
 *    (a) 利用に伴うドキュメント（利用者マニュアルなど）に，上記の著作
 *        権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 利用の形態を，別に定める方法によって，上記著作権者に報告する
 *        こと．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者を免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者は，
 *  本ソフトウェアに関して，その適用可能性も含めて，いかなる保証も行わ
 *  ない．また，本ソフトウェアの利用により直接的または間接的に生じたい
 *  かなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id: sh4a.h,v 1.2 2002/04/05 07:43:51 honda Exp $
 */

#ifndef _SH4A_H_
#define _SH4A_H_

/*
 *  SH4A(SH7780)の内部レジスタ定義
 */
typedef unsigned char   byte;
typedef volatile byte   IOREG;          
typedef unsigned short  word;
typedef volatile word   HIOREG;
typedef volatile int    LIOREG;


/*
 *  例外イベントレジスタ
 */
#define TRA	        0xff000020
#define EXPEVT	    0xff000024
#define INTEVT	    0xff000028


/*
 * 割り込みによって INTEVT レジスタに設定される値
 */
#if defined(SH7780)
#define    TMU0_INTEVT        0x580
#define    SCIF_ERI_INTEVT    0xB80
#define    SCIF_RXI_INTEVT    0xBA0
#define    SCIF_BRI_INTEVT    0xBC0
#define    SCIF_TXI_INTEVT    0xBE0
#elif defined(SH7770)
#define    TMU0_INTEVT        0x400
#define    SCIF4_INTEVT    0xA20
#define    SCIF5_INTEVT    0xA40
#define    SCIF7_INTEVT    0xA80
#define    SCIF8_INTEVT    0xAA0
#elif defined(RP1)
#define    TMU0_INTEVT        0x400
#define    SCIF_ERI_INTEVT    0x780
#define    SCIF_RXI_INTEVT    0x7A0
#define    SCIF_BRI_INTEVT    0x7C0
#define    SCIF_TXI_INTEVT    0x7E0
#elif defined(SH7751)
#define    TMU0_INTEVT        0x400
#define    SCIF_ERI_INTEVT    0x700
#define    SCIF_RXI_INTEVT    0x720
#define    SCIF_BRI_INTEVT    0x740
#define    SCIF_TXI_INTEVT    0x760
#endif


        
/*
 * 割込みコントロールレジスタ
 */
#if defined(SH7780)
#define	ICR     ((LIOREG *)(0xffd00000))	
#define	IPRA	((LIOREG *)(0xffd40000))		
#define	IPRB	((LIOREG *)(0xffd40004))		
#define	IPRC	((LIOREG *)(0xffd40008))	

#define IMSK    ((LIOREG *)(0xffd40038))
#define IMSKC   ((LIOREG *)(0xffd4003c))

#elif defined(SH7770)
#define	ICR     ((LIOREG *)(0xffd00000))	
#define	IPRA	((LIOREG *)(0xffe00000))		
#define	IPRB	((LIOREG *)(0xffe00004))		
#define	IPRC	((LIOREG *)(0xffe00008))

#define IPRG    ((LIOREG *)(0xffe00018))

#define IPRL    ((LIOREG *)(0xffe0002c))

#define IMSK    ((LIOREG *)(0xffe00040))
#define IMSKC   ((LIOREG *)(0xffe00044))

#elif defined(SH7751)
#define ICR      ((LIOREG *)(0xffd00000))
#define IPRA     ((LIOREG *)(0xffd00004))
#define IPRB     ((LIOREG *)(0xffd00008))
#define IPRC     ((LIOREG *)(0xffd0000C))

#define IPRD     ((LIOREG *)(0xffd00010))

#define IMSK     ((LIOREG *)(0xfe080040))
#define IMSKC    ((LIOREG *)(0xfe080060))


#elif defined(RP1)

#define	ICR     ((LIOREG *)(0xfe410000))	
#define	IPRA	((LIOREG *)(0xfe410800))		
#define	IPRB	((LIOREG *)(0xfe410804))		
#define	IPRC	((LIOREG *)(0xfe410808))	

#if RP1_CORE_ID == 1

 
#elif RP1_CORE_ID == 2
 
#elif RP1_CORE_ID == 3
 
#else
/* RP1_CORE_ID == 0 */
#define IMSK0    ((LIOREG *)(0xfe410820))
#define IMSK2    ((LIOREG *)(0xfe410840))
#define IMSKC0   ((LIOREG *)(0xfe410850))
#define IMSKC2   ((LIOREG *)(0xfe410870))
#define C1IMSK0    ((LIOREG *)(0xfe410824))
#define C1IMSK2    ((LIOREG *)(0xfe410844))
#define C1IMSKC0   ((LIOREG *)(0xfe410854))
#define C1IMSKC2   ((LIOREG *)(0xfe410874))
#define C2IMSK0    ((LIOREG *)(0xfe410828))
#define C2IMSK2    ((LIOREG *)(0xfe410848))
#define C2IMSKC0   ((LIOREG *)(0xfe410858))
#define C2IMSKC2   ((LIOREG *)(0xfe410878))
#define C3IMSK0    ((LIOREG *)(0xfe41082c))
#define C3IMSK2    ((LIOREG *)(0xfe41084c))
#define C3IMSKC0   ((LIOREG *)(0xfe41085c))
#define C3IMSKC2   ((LIOREG *)(0xfe41087c))
#endif

#endif

/*
 * タイマーレジスタ
 */
typedef struct{
    IOREG  TOCR;
    HIOREG dummy1;
    IOREG  TSTR;
    IOREG  dummy2;
    LIOREG TCOR0;
    LIOREG TCNT0;
    HIOREG TCR0;
    LIOREG TCOR1;
    LIOREG TCNT1;
    HIOREG TCR1;
    LIOREG TCOR2;
    LIOREG TCNT2;
    HIOREG TCR2;
} tmu;

#define	TMU_STR0	0x01
#define	TMU_STR1	0x02
#define	TMU_STR2	0x04
#define	TMU_STR3	0x01
#define	TMU_STR4	0x02
#define	TMU_STR5	0x04
#define TCR_UNF     0x0100    

#if defined(SH7780)

#define TMU (*(volatile tmu *)0xffdc0000) /* ch 3,4,5 */

#define IMSK_TMU012 0x00000001
#define IMSK_TMU345 0x00000002

#define TMU0_INTEVT 0x580
#define TMU1_INTEVT 0x5a0
#define TMU2_INTEVT 0x5c0
#define TMU3_INTEVT 0xe00
#define TMU4_INTEVT 0xe20
#define TMU5_INTEVT 0xe40

#define IPRA_TMU0(n) (*IPRA = ((*IPRA & 0x00ffffff) | (n << 24)))
#define IPRA_TMU1(n) (*IPRA = ((*IPRA & 0xff00ffff) | (n << 16)))
#define IPRA_TMU2(n) (*IPRA = ((*IPRA & 0xffff00ff) | (n << 8)))
#define IPRA_TMU3(n) (*IPRB = ((*IPRB & 0x00ffffff) | (n << 24)))
#define IPRA_TMU4(n) (*IPRB = ((*IPRB & 0xff00ffff) | (n << 16)))
#define IPRA_TMU5(n) (*IPRB = ((*IPRB & 0xffff00ff) | (n << 8)))

#elif defined(SH7770)

#define TMU (*(volatile tmu *)0xffd81000)
                              
#define IMSK_TMU012 0x00000002
#define TMU1_INTEVT 0x420
#define TMU2_INTEVT 0x440
#define TMU3_INTEVT 0x480

#define IPRA_TMU0(n) (*IPRA = ((*IPRA & 0xff00ffff) | (n << 16)))
#define IPRA_TMU1(n) (*IPRA = ((*IPRA & 0x00ffffff) | (n << 24)))
#define IPRA_TMU2(n) (*IPRA = ((*IPRA & 0xff00ffff) | (n << 16)))
#define IPRA_TMU3(n) (*IPRL = ((*IPRL & 0xffffff00) | (n << 0 )))

#elif defined(RP1)
#define TMU (*(volatile tmu *)0xffc20000)
#define TMU_SLAVE (*(volatile tmu *)0xffc10000)

#define TMU1_INTEVT 0x420
#define TMU2_INTEVT 0x440
#define TMU3_INTEVT 0x460
#define TMU4_INTEVT 0x480
#define TMU5_INTEVT 0x4A0

#define IMSK_TMU0 0x00000002
#define IMSK_TMU1 0x00000004
#define IMSK_TMU2 0x00000008
#define IMSK_TMU3 0x00000010
#define IMSK_TMU4 0x00000020
#define IMSK_TMU5 0x00000040
#define IPRA_TMU0(n) (*IPRA = ((*IPRA & 0xfffffff0) | (n << 0)))
#define IPRA_TMU1(n) (*IPRA = ((*IPRA & 0xffffff0f) | (n << 4)))
#define IPRA_TMU2(n) (*IPRA = ((*IPRA & 0xfffff0ff) | (n << 8)))
#define IPRA_TMU3(n) (*IPRA = ((*IPRA & 0xffff0fff) | (n << 12)))
#define IPRA_TMU4(n) (*IPRA = ((*IPRA & 0xfff0ffff) | (n << 16)))
#define IPRA_TMU5(n) (*IPRA = ((*IPRA & 0xff0fffff) | (n << 20)))

#elif defined(SH7751)
#define TMU (*(volatile tmu *)0xffd80000)

#define IMSK_TMU012 0x00000100
#define TMU1_INTEVT 0x420
#define TMU2_INTEVT 0x440

#define IPRA_TMU0(n) (*IPRA = ((*IPRA & 0xffff0fff) | (n << 12)))
#define IPRA_TMU1(n) (*IPRA = ((*IPRA & 0xfffff0ff) | (n << 8)))
#define IPRA_TMU2(n) (*IPRA = ((*IPRA & 0xffffff0f) | (n << 4)))

#endif

/* TSTR */
#define TSTR_STR_MASK       0x07

#define TCR_UNF_BIT	        0x0100 /* Underflow Flag 1: underflow occured. */
#define TCR_UNIE_BIT        0x0020 /* Underflow Interrupt Control. 1: enable TUNIn */

#define TCR_CKEG_MASK   	0x0018 /* Clock Edge select */
#define TCR_CKEG_UP         0x0000 /*  select Up edge */
#define TCR_CKEG_DOWN   	0x0008 /*  select Down edge */
#define TCR_CKEG_BOTH       0x0010 /*  select UP/Down both edge */

#define TCR_TPSC_MASK   	0x0007 /* Timer PreScaler. select TCNT count clock */
#define TCR_TPSC_PCK4   	0x0000 /* use Pck/4 */
#define TCR_TPSC_PCK16  	0x0001 /* use Pck/16 */
#define TCR_TPSC_PCK64  	0x0002 /* use Pck/64 */
#define TCR_TPSC_PCK256 	0x0003 /* use Pck/256 */
#define TCR_TPSC_PCK1024	0x0004 /* use Pck/1024 */
/*#define TCR_TPSC_NEVER	0x0005 */ /* NEVER SET THIS */
#define TCR_TPSC_RTCCLK 	0x0006 /* use RTCCLK (RTC output clock */
#define TCR_TPSC_EXTCLK 	0x0007 /* use TCLK (External Clock). NEVER SET CH[345] */

/*
 *
 *  FIFO付きシリアルコミュニケーションインターフェース(SCIF)レジスタ
 *
 */
#if defined(SH7780)
typedef struct{
    HIOREG SCSMR;  HIOREG dummy1;
    IOREG  SCBRR;  HIOREG dummy2;
    HIOREG SCSCR;  HIOREG dummy3;
    IOREG  SCFTDR; HIOREG dummy4;
    HIOREG SCFSR;  HIOREG dummy5;
    IOREG  SCFRDR; HIOREG dummy6;
    HIOREG SCFCR;  HIOREG dummy7;
    HIOREG SCTFDR; HIOREG dummy8;
    HIOREG SCRFDR; HIOREG dummy9;
    HIOREG SCSPTR; HIOREG dummy10;
    HIOREG SCLSR;  HIOREG dummy11;
    HIOREG SCRER;
} scif;
#elif defined(SH7770)
typedef struct{
    HIOREG SCSMR;  HIOREG dummy1;
    IOREG  SCBRR;  HIOREG dummy2;
    HIOREG SCSCR;  HIOREG dummy3;
    IOREG  SCFTDR; HIOREG dummy4;
    HIOREG SCFSR;  HIOREG dummy5;
    IOREG  SCFRDR; HIOREG dummy6;
    HIOREG SCFCR;  HIOREG dummy7;
    HIOREG SCFDR; HIOREG dummy8;
    HIOREG SCSPTR; HIOREG dummy10;
    HIOREG SCLSR;  HIOREG dummy11;
} scif;
#elif defined(RP1)
typedef struct{
    HIOREG SCSMR;  HIOREG dummy1;
    IOREG  SCBRR;  HIOREG dummy2;
    HIOREG SCSCR;  HIOREG dummy3;
    IOREG  SCFTDR; HIOREG dummy4;
    HIOREG SCFSR;  HIOREG dummy5;
    IOREG  SCFRDR; HIOREG dummy6;
    HIOREG SCFCR;  HIOREG dummy7;
    HIOREG SCTFDR; HIOREG dummy8;
    HIOREG SCRFDR; HIOREG dummy9;
    HIOREG SCSPTR; HIOREG dummy10;
    HIOREG SCLSR;  HIOREG dummy11;
} scif;
#elif defined(SH7751)
typedef struct{
    HIOREG SCSMR;  HIOREG dummy1;
    IOREG  SCBRR;  HIOREG dummy2;
    HIOREG SCSCR;  HIOREG dummy3;
    IOREG  SCFTDR; HIOREG dummy4;
    HIOREG SCFSR;  HIOREG dummy5;
    IOREG  SCFRDR; HIOREG dummy6;
    HIOREG SCFCR;  HIOREG dummy7;
    HIOREG SCFDR; HIOREG dummy8;
    HIOREG SCSPTR; HIOREG dummy10;
    HIOREG SCLSR;  HIOREG dummy11;
} scif;
#endif

#if defined(SH7780)
#if 0 /* SCIF 0 */
#define SCIF (*(volatile scif *)0xffe00000)
#define INTR_CLEAR_SCIF  (0x1 << 3)
#define IPRC_SCIF(n) (*IPRC = ((*IPRC & 0x00ffffff) | (n << 24)))
#undef SCIF_ERI_INTEVT
#undef SCIF_RXI_INTEVT
#undef SCIF_BRI_INTEVT
#undef SCIF_TXI_INTEVT
#define SCIF_ERI_INTEVT 0x700
#define SCIF_RXI_INTEVT 0x720
#define SCIF_BRI_INTEVT 0x740
#define SCIF_TXI_INTEVT 0x760

#else /* SCIF 1 */
#define SCIF (*(volatile scif *)0xffe10000)
#define INTR_CLEAR_SCIF  (0x1 << 4)
#define IPRC_SCIF(n) (*IPRC = ((*IPRC & 0xff00ffff) | (n << 16)))
#endif

#elif defined(SH7770)
#define SCIF4 (*(volatile scif *)0xff927000)
#define SCIF5 (*(volatile scif *)0xff928000)
#define IMSK_SCIF  0x02000000
#define IPRC_SCIF4(n) (*IPRG = ((*IPRG & 0x00ffffff) | (n << 24)))
#define IPRC_SCIF5(n) (*IPRG = ((*IPRG & 0xff00ffff) | (n << 16)))
#define IPRC_SCIF7(n) (*IPRG = ((*IPRG & 0xff00ffff) | (n << 16)))
#define IPRC_SCIF8(n) (*IPRG = ((*IPRG & 0xffff00ff) | (n << 8)))

#elif defined(SH7751)
#define SCIF (*(volatile scif *)0xffe80000)
#define INTR_CLEAR_SCIF (0x1 << 4)
#define IPRC_SCIF(n) (*IPRC = ((*IPRC & 0xffffff0f) | (n << 4)))

#elif defined(RP1)

#if 0 /* channel 0 */
#define SCIF (*(volatile scif *)0xffc30000)
#define IMSK_SCIF  0x0000000f
#define IPRC_SCIF(n) (*IPRB = ((*IPRB & 0xfffffff0) | (n << 0)))
#undef SCIF_ERI_INTEVT
#undef SCIF_RXI_INTEVT
#undef SCIF_BRI_INTEVT
#undef SCIF_TXI_INTEVT
#define SCIF_ERI_INTEVT 0x700
#define SCIF_RXI_INTEVT 0x720
#define SCIF_BRI_INTEVT 0x740
#define SCIF_TXI_INTEVT 0x760
#else /* channel 1 */
#define SCIF (*(volatile scif *)0xffc40000)
#define IMSK_SCIF  0x000000f0
#define IPRC_SCIF(n) (*IPRB = ((*IPRB & 0xffffff0f) | (n << 4)))
#endif

#endif

#define	SCSMR_CHR	  0x0040
#define	SCSMR_PE	  0x0020
#define	SCSMR_OE	  0x0010
#define	SCSMR_STOP	  0x0008
#define	SCSMR_CKS1	  0x0002
#define	SCSMR_CKS0	  0x0001

#define SCSCR_TIE	  0x0080
#define SCSCR_RIE	  0x0040
#define SCSCR_TE	  0x0020
#define SCSCR_RE	  0x0010
#define SCSCR_CKE1	  0x0002
#define SCSCR_CKE0	  0x0001

#define SCFSR_ER	  0x0080
#define SCFSR_TEND	  0x0040
#define SCFSR_TDFE	  0x0020
#define SCFSR_BRK	  0x0010
#define SCFSR_FER	  0x0008
#define SCFSR_PER	  0x0004
#define SCFSR_RDF	  0x0002
#define SCFSR_DR	  0x0001


#define SCFCR_RTRG1  0x0080
#define SCFCR_RTRG0  0x0040
#define SCFCR_TTRG1  0x0020
#define SCFCR_TTRG0  0x0010
#define SCFCR_MCE	 0x0008
#define SCFCR_TFRST  0x0004
#define SCFCR_RFRST  0x0002
#define SCFCR_LOOP	 0x0001


#define SCSPTR_RTSIO  0x0080
#define SCSPTR_RTSDT  0x0040
#define SCSPTR_CTSIO  0x0020
#define SCSPTR_CTSDT  0x0010
#define SCSPTR_SPB2IO 0x0002
#define SCSPTR_SPB2DT 0x0001

#define SCSSR_ER	0x0080
#define SCSSR_TEND	0x0040
#define SCSSR_TDFE	0x0020
#define SCSSR_BRK	0x0010
#define SCSSR_FER	0x0008
#define SCSSR_PER	0x0004
#define SCSSR_RDF	0x0002
#define SCSSR_DR	0x0001



/*
 * キャッシュ制御レジスタ
 */
#define	CCR	((LIOREG *)0xff00001c)	

#define SR_BL_DISABLE   0x10000000
#define SR_BL_ENABLE    0xefffffff
#define SR_BL_MASK      0x10000000
#define SR_MDRBBL_MASK  0x70000000
#define SR_IMASK_MASK   0x000000f0
#define SR_IMASK_CLEAR  0xffffff0f
#define SR_RB_MASK		0x20000000

#define SCIF_BPS_VALUE  13

#if defined(SH7751)
  #define MAX_EVT 0xb80
#elif defined(SH7780)
  #define MAX_EVT 0xfe0 
#elif defined(RP1)
  #define MAX_EVT 0xfe0 
#endif

#define VECTOR_SIZE (MAX_EVT >> 5)

#define TLB_R_EXPEVT 0x040
#define TLB_W_EXPEVT 0x060
#define PERM_R_EXPEVT 0x0a0
#define PERM_W_EXPEVT 0x0c0
#define ADDR_R_EXPEVT 0x0e0
#define ADDR_W_EXPEVT 0x100
#define SYSCALL_EXPEVT 0x160

inline static uint read_sr()
{
  uint sr;
  asm volatile(
      "stc sr, %0"
      :"=r"(sr)
      );
  return sr;
}

inline static void load_sr(uint val)
{
  asm volatile(
      "ldc %0, sr"
      :
      :"r"(val)
      );
}

inline static void cli(void)
{
  unsigned long __srval;
  asm volatile(
      "stc   sr,%0\n\t"
      "or    %1,%0\n\t"
      "ldc   %0,sr\n\t"
      :"+&z"(__srval)
      :"r"(SR_BL_DISABLE)
      );

}

inline static void sti(void)
{
  unsigned long __srval;
  asm volatile(
      "stc  sr,%0\n\t"
      "and   %1,%0\n\t"
      "ldc   %0,sr\n\t"
      :"+&z"(__srval)
      :"r"(SR_BL_ENABLE)
      );
}

#endif
