/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/cpureg.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:39 $
 */

#ifndef	__CPUREG__

#define	__CPUREG__

/*
** cpureg.h
**	This file contains all the definitions for the things specific
**	to the IP2.
**
**	Addresses of the various onboard registers and devices. They
**	are only accessible in supervisor mode via the system segment, 3.
*/
#ifndef LOCORE

	/* stack limit reg	(short)	*/
#	define	STKLMT_REG	((u_short *)0x3f000000)

	/* stack base reg	(short)	*/
#	define	STKBASE_REG	((u_short *)0x3e000000)

	/* text/data limit reg	(short)	*/
#	define	TDLMT_REG	((u_short *)0x3d000000)

	/* text/data base reg	(short)	*/
#	define	TDBASE_REG	((u_short *)0x3c000000)

	/* page table map base	(long)	*/
#	define	PTMAP_BASE	0x3b000000

	/* multibus protection	(char)	*/
#	define	MBP_REG		((u_char *)0x3a000000)

	/* parity control	(char)	*/
#	define	PARCTL_REG	((u_char *)0x39000000)

	/* status register	(short)	*/
#	define	STATUS_REG	((u_short *)0x38000000)

	/* os base		(char)	*/
#	define	OS_BASE		((u_char *)0x36000000)

	/* clock data register	(char)	*/
#	define	CLK_DATA	((u_char *)0x35000000)

	/* clock control reg	(char)	*/
#	define	CLK_CTL		((u_char *)0x34000000)

	/* page fault clear reg	(char)	*/
#	define	PGFLTCLR_REG	((u_char *)0x33800000)

	/* static ram base: 2kb worth	*/
#	define	SRAM_BASE	0x33000000

	/* base addr for duart1 (tty2-3)*/
#	define	DUART1_BASE	0x32800000

	/* base addr for duart0	(tty0-1)*/
#	define	DUART0_BASE	0x32000000

	/* switch register addr	(short)	*/
#	define	SWTCH_REG	((u_short *)0x31800000)

	/* mousey quadrature	(short)	*/
#	define	M_QUAD		(u_short *)0x31000000

	/* mousey buttons	(char)	*/
#	define	M_BUT		((u_char *)0x30800000)

	/* PROM base addr		*/
#	define	PROM_BASE	0x30000000

#else

	/* stack limit reg	(short)	*/
#	define	STKLMT_REG	0x3f000000

	/* stack base reg	(short)	*/
#	define	STKBASE_REG	0x3e000000

	/* text/data limit reg	(short)	*/
#	define	TDLMT_REG	0x3d000000

	/* text/data base reg	(short)	*/
#	define	TDBASE_REG	0x3c000000

	/* page table map base	(long)	*/
#	define	PTMAP_BASE	0x3b000000

	/* multibus protection	(char)	*/
#	define	MBP_REG		0x3a000000

	/* parity control	(char)	*/
#	define	PARCTL_REG	0x39000000

	/* status register	(short)	*/
#	define	STATUS_REG	0x38000000

	/* os base		(char)	*/
#	define	OS_BASE		0x36000000

	/* clock data register	(char)	*/
#	define	CLK_DATA	0x35000000

	/* clock control reg	(char)	*/
#	define	CLK_CTL		0x34000000

	/* page fault clear reg	(char)	*/
#	define	PGFLTCLR_REG	0x33800000

	/* static ram base: 2kb worth	*/
#	define	SRAM_BASE	0x33000000

	/* base addr for duart1 (tty2-3)*/
#	define	DUART1_BASE	0x32800000

	/* base addr for duart0	(tty0-1)*/
#	define	DUART0_BASE	0x32000000

	/* switch register addr	(short)	*/
#	define	SWTCH_REG	0x31800000

	/* mousey quadrature	(short)	*/
#	define	M_QUAD		0x31000000

	/* mousey buttons	(char)	*/
#	define	M_BUT		0x30800000

	/* PROM base addr		*/
#	define	PROM_BASE	0x30000000

#endif LOCORE

/*
** Virtual spaces
**
** we have a switch selectable kernel address space: 2 or 4mb.
** We limit that the actual kernel+data+bss size is <= 1mb.
** The second 1mb of this area is used for virtual addresses to map
**    various things.
** The possible remaining 2mb are not currently used.
*/


/* kernel text+data+bss: base & limit */
#define	KERN_VBASE	0x20000000	/* base address of kernel	*/
#define	KERN_VLIMIT	0x20200000	/* end of kernel + 1		*/
#define	KERN_VSIZE	0x200000	/* size in bytes of kernel	*/

/*
 * User page table area.  This virtual space is used for several things.
 * The user page tables, the per process data area (udot/struct user),
 * and the virtual cache memory are allocated from this space.  In
 * the case of the virtual cache, the mapping is transitory, and doesn't
 * need to be permanent.  The 0th entry in this space is used for
 * process 0's page table.
 */
#define	USRPT_VBASE	0x20200000	/* 1024k+896k of usrpt space	*/
#define	USRPT_VLIMIT	0x203E0000	/* end of usrpt area + 1	*/

/* shared memory (between graphics and kernel) */
#define	SHMEM_VBASE	0x203F0000	/* shared memory base		*/
#define	SHMEM_VLIMIT	0x203F2000	/* end of shared memory area + 1*/


/*
** multibus memory space
**  The multibus memory occupies a 2mb virtual space.  The lower megabyte
**  is the mapped memory with the high megabyte the multibus mapping
**  registers.
*/
#define	MBUF_VBASE	0x40000000	/* 1mb of mbuf space		*/
#define	MBUF_VLIMIT	0x40100000	/* end of mbuf area + 1		*/

#define	MBREG_VBASE	0x40100000	/* 1mb of multibus registers	*/
#define	MBREG_VLIMIT	0x40200000	/* end of this area + 1		*/

/* multibus i/o space */
#define	MBIO_VBASE	0x50000000	/* 64k of multibus i/o		*/
#define	MBIO_VLIMIT	0x50010000	/* end of multibus i/o + 1	*/

#define	UDOT_VBASE	0x203FF000	/* user structure		*/
#define	UDOT_VLIMIT	0x20400000	/* end of user struct + 1	*/

/* misc virtual spaces */
#define	DEVMEM_VBASE	0x203F2000	/* used by /dev/{k,}mem		*/
#define	MBUS_VBASE	0x203F3000	/* used by multibus map code	*/
#define	SCRPG0_VBASE	0x203F4000	/* scratch page #0		*/
#define	SCRPG1_VBASE	0x203F5000	/* scratch page #1		*/
#define	SCRPG2_VBASE	0x203F6000	/* scratch page #2		*/
#define	SCRPG3_VBASE	0x203F7000	/* scratch page #3		*/
#define	FORKUTL_VBASE	0x203F8000	/* fork udot			*/
#define	XSWAPUTL_VBASE	0x203F9000	/* xswap udot			*/
#define	XSWAP2UTL_VBASE	0x203FA000	/* xswap2 udot			*/
#define	SWAPUTL_VBASE	0x203FB000	/* swap udot			*/
#define	PUSHUTL_VBASE	0x203FC000	/* page push udot		*/
#define	VFUTL_VBASE	0x203FD000	/* vfork udot			*/

/*
 * Static multibus memory for brane damaged controllers.
 * WUB_VBASE is the pmII's address of a page of memory stashed for driver
 * usage.  WUB_MBADDR is the address of the same page, from the multibus's
 * point of view.  WUB_MBLIMIT is the end of the WUB area, plus one.
 */
#define	WUB_VBASE	0x203FE000	/* wub page */
#define	WUB_MBADDR	0x07F000	/* for the dsd controller */
#define	WUB_MBLIMIT	0x080000	/* one page, please */

/*
** definitions for the various segments that are present.  A segment
** is a "virtual address space".  Segments 0x7 to 0xE are currently
** unused.
*/
#define	SEG_MSK		0xf0000000	/* mask for segment bits only	*/
#define	SEG_TD		0x0		/* text/data segment		*/
#define	SEG_STK		0x10000000	/* stack segment		*/
#define	SEG_OS		0x20000000	/* kernel segment		*/
#define	SEG_SYS		0x30000000	/* system segment (proms)	*/
#define	SEG_MBMEM	0x40000000	/* multibus memory segment	*/
#define	SEG_MBIO	0x50000000	/* multibus i/o segment		*/
#define	SEG_GE		0x60000000	/* geometry pipe segment	*/
#define	SEG_FPA		0xF0000000	/* floating point accel segment	*/

/*
** definitions for the parity control register.  Each bit indicates
** if parity should be checked during the associated cycle
*/
#define	PAR_UR		0x1	/* check parity on user reads		*/
#define	PAR_UW		0x2	/*      "          user writes		*/
#define	PAR_KR		0x4	/*      "          kernel reads		*/
#define	PAR_KW		0x8	/*      "          kernel writes	*/
#define	PAR_DIS0	0x10	/* disable driving duart0 and LEDs	*/
#define	PAR_DIS1	0x20	/* disable driving duart1		*/
#define	PAR_MBR		0x40	/* check parity on multibus reads	*/
#define	PAR_MBW		0x80	/*      "          multibus writes	*/

/*
** definitions for the various bits in the status register
**  bits 14 and 15 are not currently used.
**  Trailing _ means active low.
*/
#define	ST_EQKTIMO	0x8000	/* enable quick timeout on accesses	*/
#define	ST_EWDOG	0x4000	/* enable watchdog timeout		*/
#define	ST_BADPAR	0x2000	/* generate incorrect parity		*/
/*
 * NOTE: the next two defines are the same.  For revision B boards a 4
 * Meg space is always used and the pipe is optionally driven
*/
#define	ST_OSSIZE	0x1000	/* size of kernel space (2M/4M)	(Rev A) */
#define	ST_DISGE	0x1000	/* disable driving the pipe (Rev B)	*/
#define	ST_ECBRQ	0x800	/* hold bus until CBRQ asserted		*/
#define	ST_IP2ACC	0x400	/* IP2 access from multibus allowed	*/
#define	ST_GEUACC	0x200	/* user access to GE allowed		*/
#define	ST_FPAUACC	0x100	/* user access to fpa allowed		*/
#define	ST_SYSSEG_	0x80	/* allow access to system seg only	*/
#define	ST_MBINIT	0x40	/* force a multibus init		*/
#define	ST_EINTR	0x20	/* enable interrupts			*/
#define	ST_EXINTR	0x10	/* enable external interrupts		*/
#define	ST_LEDMSK	0xf	/* mask for just the led bits		*/
#define	ST_LEDB3	0x8	/* led bits 3 thru 0			*/
#define	ST_LEDB2	0x4
#define	ST_LEDB1	0x2
#define	ST_LEDB0	0x1

/*
** The current layout for the switch register
**		      8  7  6 5    4321 - backpanel
**		      1  2  3 4    5678 - S1
** 1    23   45  678 - S2
**15               8                  0
** X    XX   XX  XXX  X  X  X  X   XXXX
** |    |    |    |   |  |  |  |    +---	Boot type
** |    |    |    |   |  |  |  |
** |    |    |    |   |  |  |  +--------	Autoboot mode
** |    |    |    |   |  |  |
** |    |    |    |   |  |  +-----------	Quite mode
** |    |    |    |   |  |
** |    |    |    |   |  +--------------	Secondary display
** |    |    |    |   |
** |    |    |    |   +-----------------	Reserved
** |    |    |    |
** |    |    |    +---------------------	display type  combinations
** |    |    |
** |    |    +--------------------------	RS232 console speed
** |    |
** |    +-------------------------------	Reserved
** |
** +---------------------------------------     Master/Slave
**
** definitions/masks for the various bits in the switch register
*/
#define	SW_MASTRSLV	0x8000	/* master/slave and which 2mb multibus	*/
				/* space occupies			*/
#define	SW_RESVH	0x6000	/* Reserved bits 			*/
#define	SW_CONSSPD	0x1800	/* rs232 console speed			*/
#define	SW_DISCOMBO	0x0700	/* Display combinations			*/
#define	SW_RESVL	0x0080	/* reserved				*/
#define	SW_2DIS		0x0040	/* use secondary display in booting	*/
#define	SW_QUITE	0x0020	/* hush hush				*/
#define	SW_AUTOBT	0x0010	/* autoboot or stop in prom monitor	*/
#define	SW_BTTYPE	0x000F	/* boot type				*/

/* Boot types:		*/
#define BT_HD		0x00	/* hard disk (try ip, sd, then md	*/
#define BT_TAPE		0x01	/* cartridge tape			*/
#define BT_FD		0x02	/* floppy disk (try sf, then mf		*/
#define BT_XNS		0x03	/* boot from ethernet using XNS		*/
#define BT_MONITOR	0x05	/* don't autoboot come up in monitor	*/
#define BT_ROM		0x06	/* boot from prom board 		*/
/* these defines are to force using a specific controller		*/
#define BT_IP		0x09	/* Interphase SMD disk boot		*/
#define BT_ST		0x0A	/* Storager tape boot			*/
#define BT_SF		0x0B	/* Storager tape boot			*/
#define BT_SD		0x0C	/* Storager hard disk boot		*/
#define BT_MT		0x0D	/* DSD tape boot			*/
#define BT_MF		0x0E	/* DSD tape boot			*/
#define BT_MD		0x0F	/* DSD hard disk boot			*/

/*	RS232 console (ie port2) speed					*/
#define CON_9600	0x00	/* 9600 baud				*/
#define CON_300		0x01	/* 300 baud				*/
#define CON_1200	0x02	/* 1200 baud				*/
#define CON_192		0x03	/* 19.2K baud				*/
#define CON_600		0x04	/* 600 baud (note not possible setting	*/

/*	Display combination types: (Primary_Secondary)			*/
/*	NI - non-interlaced 60HZ, I - interlaced 30 HZ, NTSC - RS 170A  */
/*	EU - European TV (bad) or PAL					*/
/* NOTE: don't change the order without looking at gl/gl2gl.c prom code */
#define	DS_NI_NI	0x00
#define	DS_NI_I		0x01
#define	DS_NI_NTSC	0x02
#define	DS_NI_EU	0x03
#define	DS_I_NI		0x04
#define	DS_I_I		0x05
#define	DS_I_NTSC	0x06
#define	DS_I_EU		0x07

#define DIS_NI		0
#define DIS_I		1
#define DIS_NTSC	2
#define DIS_EU		3

#ifndef LOCORE
/*
** a cute struct to rip apart the switch register
*/
struct swregbits
{
	unsigned short
			sw_mstrslv	: 1,
			sw_resvh	: 2,
			sw_consspd	: 2,
			sw_discombo	: 3,
			sw_resvl	: 1,
			sw_secdis	: 1,
			sw_quite	: 1,
			sw_autobt	: 1,
			sw_bttype	: 4
};
#endif

/*
** mouse quadrature layout
*/
#define	MOUSE_XFIRE	0x0100		/* if low, then x quadrature fired */
#define	MOUSE_XCHANGE	0x0200		/* if low then x--, else x++	   */
#define	MOUSE_YFIRE	0x0400		/* if low, then y quadrature fired */
#define	MOUSE_YCHANGE	0x0800		/* if low then y--, else y++	   */
#define	MOUSE_XFIREBIT_	8		/* bit number of x fire bit	   */
#define	MOUSE_XBIT	9		/* bit number of x change bit	   */
#define	MOUSE_YFIREBIT_	10		/* bit number of y fire bit	   */
#define	MOUSE_YBIT	11		/* bit number of y change bit	   */

/*
** mouse button layout
*/
#define	MBUT_RIGHT	0x01		/* right button		*/
#define	MBUT_MIDDLE	0x02		/* middle button	*/
#define	MBUT_LEFT	0x04		/* left button		*/
#define REV2		0x10		/* board revision #1 or #2	*/
#define REVSHIFT	5		/* top 3 bits for revision above 3 */
#define REVLEVEL	((*M_BUT) & REV2) ? 2 : (((*M_BUT) >> REVSHIFT) + 3)

/*
** Graphics ports
*/
#define	_GEPORT_	0x60001000
#define	_GETOKEN_	0x60000000


/*
** definitions for the various bits in the multibus protection register
*/
#define	MBP_HMACC	0x80	/* allow upper mem access (megs 8-f)	*/
#define	MBP_LMACC	0x40	/* allow lower mem access (megs 0-7)	*/
#define	MBP_HIOACC	0x20	/* allow upper io access  (megs 8-f)	*/
#define	MBP_LIOACC	0x10	/* allow lower io access  (megs 0-7)	*/
#define	MBP_DMACC	0x8	/* allow GL2 DMA access   (megs 8,9,A,B)*/
#define	MBP_GFACC	0x4	/* allow GF board access  (io page 1)	*/
#define	MBP_UCACC	0x2	/* update controller acc  (io page 3)	*/
#define	MBP_DCACC	0x1	/* display controller acc (io page 4)	*/

/*
** misc definitions
*/
#define	SRAM_SZ		0x800	/* size of the static ram (2kb)	*/
#define	PTMAP_SZ	0x4000	/* size of the page table map	*/
#define	CLK_DATASZ	50	/* size of the clock data ram	*/
#if 0
#define	GFXCONSOLE	0	/* arg to coswitch(), switch to	*/
				/* gfx console if possible	*/
#define	SECCONSOLE	1	/* arg to coswitch(), switch to	*/
				/* debug console		*/
#endif

#ifndef LOCORE

/*
** Some handy dandy macros for accessing the above registers
**
** output a value to the leds -- note that leds require active low.
*/
#define	LEDS(x)	*STATUS_REG &= ~ST_LEDMSK; *STATUS_REG |= (~x)&ST_LEDMSK;

/*
** For compatability we need to define a macro to map kernel virtual
** addresses to physical addresses.  Currently this is very simple:
** The 1st 1/2mb of the kernel maps 1-to-1 virtual to physical.
** So all we need to do is remove the "kernel segment number" from the
** virtual address and **POOF** we have the physical address.
** XXX - this should change!
*/
extern int	mbwin;

/* #define	VTOP(a)		(((int)(a) & ~SEG_MSK) + mbwin) */

#endif LOCORE

#endif	__CPUREG__
