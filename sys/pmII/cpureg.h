/*
 * Definitions for the pmII cpu
 * This file defines the on-board registers as well as the virtual
 * memory addressing constants
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/cpureg.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:31 $
 */

#ifndef	__CPUREG__

#define	__CPUREG__

/*
 * Address of graphics ports
 */
#define	_GETOKEN_	0xFD4000
#define	_GEPORT_	0xFD5000

/*
 * Bits defining the pm2 status register info
 */
#define	STATUS		0xFC9000
#define	STS_LEDS	0x000F		/* 4 bits -> hex led display */
#define	STS_MAILBOX	0x0010		/* enable mailbox ints */
#define	STS_PARITY	0x0020		/* enable parity detection */
#define	STS_MBINIT	0x0040		/* init multibus */
#define	STS_NOTBOOT	0x0080		/* 0 -> boot state */
#define	STS_EN0		0x0100		/* enable external multibus mem use */
#define	STS_EN1		0x0200		/* enable external multibus mem wrt */
#define	STS_GEUSE	0x0400		/* users can use the ge port */
#define	STS_PPUSE	0x0800		/* users can use the parallel port */

/*
 * Bits defining the pm2 exception register info
 */
#define	EREG		0xFCA000
#define	ER_PAGEFAULT	0x01		/* page fault (active low) */
#define	ER_MAPFAULT	0x02		/* map fault (active low) */
#define	ER_TIMEOUT	0x04		/* memory timeout (active low) */
#define	ER_PARITY	0x08		/* parity error (active low) */
#define	ER_MOUSE	0x10		/* level4 from mouse buttons */
#define	ER_MAILBOX	0x20		/* level4 from multibus mailbox */
#define	ER_RCV		0x40		/* level4 from parallel port recieve */
#define	ER_XMIT		0x80		/* level4 from parallel port transmit */

/*
 * Mouse quadrature and button bits
 */
#define	MOUSE		0xFCC000	/* mouse quadrature */
#define	MOUSE_XFIRE	0x01		/* if low, then x quadrature fired */
#define	MOUSE_XCHANGE	0x02		/* if low then x--, else x++ */
#define	MOUSE_YFIRE	0x04		/* if low, then y quadrature fired */
#define	MOUSE_YCHANGE	0x08		/* if low then y--, else y++ */
#define	MOUSE_XFIREBIT	0		/* bit number of x fire bit */
#define	MOUSE_XBIT	1		/* bit number of x change bit */
#define	MOUSE_YFIREBIT	2		/* bit number of y fire bit */
#define	MOUSE_YBIT	3		/* bit number of y change bit */
#define	MBUT		0xFCE000	/* mouse buttons */
#define	MBUT_RIGHT	0x01		/* right button */
#define	MBUT_MIDDLE	0x02		/* middle button */
#define	MBUT_LEFT	0x04		/* left button */

/*
 * On board duarts.  The first duart's counter/timer is used for the memory
 * refresh timing, and is set to generate a 15us cycle.  The second counter
 * timer is used for the system clock.  See dureg.h for more info.
 */
#define	DUART0_VBASE	0xFC4000
#define	DUART1_VBASE	0xFC6000

/*
 * config register - switch 1 is LSB, 'open' = set, only eight switches read
 */
#define CONFIG_REG	0xFD0000		/* 8 bits of dipswitch */
#define	HOSTSPEEDMASK	0x03	/* Select host baud rate */
#define	VERBOSEMASK	0x04	/* Verbose/Checkout */
#define HOSTSPEED(n)	((n)&HOSTSPEEDMASK)
#define HOST300		0
#define HOST1200	1
#define HOST19200	2
#define HOST9600	3
#define VERBOSE(n)	((n)&VERBOSEMASK)
#define BOOTENVMASK	0xf8	/* Boot Environment */
#define	BOOTENVSHIFT	3	/* Shift right to word align */
#define	BOOTENV(n)	(((n)&BOOTENVMASK)>>BOOTENVSHIFT)

/*
 *	1  2  3  4  5  6  7  8  9  10 (ON PM1 Switch Settings)
 *	   1  2  3  4  5  6  7  8     (ON PM2 Switch Settings)
 *      .  .  .  .  .  .  .  .  .  .
 *			     X  X --> HostSpeed (Select host baud rate)
 *			  X  -------> Checkout/Verbose  (PM1)
 *	   X  X  X  X  X  ----------> BootEnv (Selects default startup mode)
 *	    	       X -----------> Supress Autoboot (using Bootenv)
 *	X   ------------------------> TTL/RS-423 signal level (always closed)
 *      .  .  .  .  .  .  .  .  .  .
 *      -  9  8  7  6  5  4  3  2  1  (ON Back Panel for PM1)
 *      -  8  7  6  5  4  3  2  1  -  (ON Back Panel for PM2)
 *         7  6  5  4  3  2  1  0     (bit numbers internally (pm2) (0=lsb))
 */

#define	ENV_NOBOOT	0x01		/* Suppress auto boot */
#define ENV_FLOPPYBOOT	0x00		/* Boot from floppy disk */
#define ENV_MONITOR	0x02		/* Enter Iris Monitor */
#define ENV_NETBOOT	0x04		/* Boot using SGI Xns boot protocol */
#define ENV_TERMULATE	0x06		/* Enter Serial Terminal Emulation */
#define ENV_NETBOOT0    0x08		/* Default netboot 0 */
#define ENV_TAPEBOOT 	0x0a		/* boot from tape */
#define ENV_488BOOT	0x0c		/* Boot using SGI 488 boot protocol */
#define ENV_DISKBOOT    0x10		/* Boot from disk */
#define ENV_SMDBOOT	0x12		/* boot from smd disk */
#define AUTOBOOT(n)	(!(n & ENV_NOBOOT))

#define ENV_RPT_PGTEST	0x17		/* Repeat page map test on boot */
#define ENV_RPT_PTTEST	0x18		/* Repeat protection map test on boot */
#define ENV_RPT_CXTEST	0x19		/* Repeat context reg. test on boot */
#define ENV_RPT_DUARTST	0x1a		/* Repeat DUART test on boot */
#define ENV_RPT_TIMETST	0x1b		/* Repeat DUART timer tests on boot */
#define ENV_RPT_RAMBTST	0x1c		/* Repeat RAM boot page tests on boot */
/*  NOTE - if the bit value for ENV_SLAVE or ENV_DONTTOUCH is changed,
	it must be changed in the pm2.1start.s file - GB */
#define ENV_SLAVE	0x1d		/* pm2 board is slave processor */
#define ENV_DIAGNOSTICS	0x1e		/* Diagnostics mode */

#define ENV_DONTTOUCH	0x1f		/* Special debugging env. only
					   valid if VERBOSE set */
#define DONTTOUCH(n)	(BOOTENV(n)==ENV_DONTTOUCH&&VERBOSE(n))

/*
 * Virutal spaces
 */

/* kernel text+data+bss+valloc base & limit */
#define	KERN_VBASE	0xC00000	/* base address of kernel */
#define	KERN_VLIMIT	0xD00000	/* end of kernel + 1 */
#define	KERN_VSIZE	0x100000	/* size in bytes of kernel */

/*
 * User page table area.  This virtual space is used for several things.
 * The user page tables, the per process data area (udot/struct user),
 * and the virtual cache memory are allocated from this space.  In
 * the case of the virtual cache, the mapping is transitory, and doesn't
 * need to be permanent.
 */
#define	USRPT_VBASE	0xD00000	/* 832k of space */
#define	USRPT_VLIMIT	0xDD0000	/* end of usrpt area + 1 */

/* shared memory (between graphics and kernel) */
#define	SHMEM_VBASE	0xDD0000	/* shared memory base */
#define	SHMEM_VLIMIT	0xDD2000	/* end of shared memory area + 1 */

/* other */
#define	DEVMEM_VBASE	0xDD2000	/* used by /dev/{k,}mem */
#define	MBUS_VBASE	0xDD3000	/* used by multibus map code */
#define	SCRPG0_VBASE	0xDD4000	/* scratch page #0 */
#define	SCRPG1_VBASE	0xDD5000	/* scratch page #1 */
#define	SCRPG2_VBASE	0xDD6000	/* scratch page #2 */
#define	SCRPG3_VBASE	0xDD7000	/* scratch page #3 */
#define	FORKUTL_VBASE	0xDD8000	/* fork udot */
#define	XSWAPUTL_VBASE	0xDD9000	/* xswap udot */
#define	XSWAP2UTL_VBASE	0xDDA000	/* xswap2 udot */
#define	SWAPUTL_VBASE	0xDDB000	/* swap udot */
#define	PUSHUTL_VBASE	0xDDC000	/* page push udot */
#define	VFUTL_VBASE	0xDDD000	/* vfork udot */

/*
 * Static multibus memory for brane damaged controllers.
 * WUB_VBASE is the pmII's address of a page of memory stashed for driver
 * usage.  WUB_MBADDR is the address of the same page, from the multibus's
 * point of view.  WUB_MBLIMIT is the end of the WUB area, plus one.
 */
#define	WUB_VBASE	0xDDE000	/* wub page */
#define	WUB_MBADDR	0x07F000	/* for the dsd controller */
#define	WUB_MBLIMIT	0x080000	/* one page, please */

/* multibus i/o */
#define	MBIO_VBASE	0xDE0000	/* 64k of multibus i/o */
#define	MBIO_VLIMIT	0xDF0000	/* end of multibus i/o + 1 */

/*
 * These address's are special.  They MUST be the same in both the users
 * virtual space and the kernels virtual space.  Don't change them unless
 * you really know what you are doing
 */
#define	IVEC_VBASE	0xDFE000	/* interrupt vectors */
#define	UDOT_VBASE	0xDFF000	/* user structure */

/*
 * Page and protection map stuff
 */

/*
 * Context register:
 *	- there are 256 context's on the pmII cpu, of which certain of them
 *	  are pre-allocated to hold the kernel mapping
 */
#define CONTEXT		0xFC8001	/* byte version of context register */
#define	WCONTEXT	0xFC8000	/* word version */

/*
 * Page map:
 *	- the page map contains the virtual to physical translation info
 */
#define PAGEBASE	0xFC0000	/* virtual base of page map */
#define NPAGEMAP	4096		/* page map size */

/*
 * Protection map:
 *	- the protection map contains the access control info for each
 *	  virtual page
 */
#define PROTBASE	0xFC2000	/* virtual base of prot map */
#define NPROTMAP	4096		/* prot map size */

/* bits found in the protection map */
#define	PR_KR		0x0100		/* kernel read only */
#define	PR_KW		0x0200		/* kernel read-write */
#define	PR_UXKW		0x0300		/* user exec, kernel read-write */
#define	PR_URKW		0x0400		/* user read, kernel read-write */
#define	PR_UWKW		0x0500		/* user-kernel read-write */
#define PR_PROTMASK	0x0F00		/* mask for above protection bits */
#define PR_ASMBRAM	0x0000		/* page points to multibus ram */
#define PR_ASMBIO	0x1000		/* page points to multibus i/o */
#define	PR_ASOBRAM	0x2000		/* page points to onboard ram */
#define PR_ASINVAL	0x3000		/* page points to nowhere */
#define	PR_ASMASK	0x3000		/* mask for above address bits */
#define PR_DIRTY	0x4000		/* page has been written */
#define PR_USED		0x8000		/* page has been read */
#define	PR_INVALID	0x3000		/* inaccessablie page */

#ifndef	LOCORE
short	beprint;		/* set to 1 to enable bus error noise */
short	unixend;		/* last page frame used by unix */
#endif

#endif	__CPUREG__
