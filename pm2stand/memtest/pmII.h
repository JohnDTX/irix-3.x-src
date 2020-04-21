/*
 * PMII Definitions
 *	All addresses A also appear at A-0xfc0000 during boot
 */
#include "pm2.1.h"
#define PROTEND		(short *)0xfc4000	/* end of prot map */
#define CONFIG_REG	(*CONFIG )		/* 8 bits of dipswitch */
#define GE		(*GEport)		/* Graphics pipeline */
#define MOUSE_BUT	MBUT			/* Mouse buttons, IPL4 */

#define CONTEXT_REG	(*CONTEXT)		/* Context: write only */
#define STATUS_REG	(*STATUS)		/* Status: read/write only */

#define DUART0		UART0			/* First DUART, IPL5 */
#define DUART1		UART1			/* Second DUART, IPL6 */
/*#define GPIB		((char *) 0xff7000)	/* IEEE 488, IPL5 */
/*#define MOUSE_Q	((char *) 0xff7801)	/* mouse quadrature IPL7 */

/*
 * Page and Protection maps
 *	Page map and protection map offsets are calculated by
 *		(context<<4) | (A>>12)
 *	The current context must match the protection map context, and
 *	 the protection must allow the current read/write/execute operation
 */

/*
 * PROTMAP: udmM pPPP cccc cccc (binary)
 *	u = used, d = dirty
 *	mM = mem type: 00->Multibus I/O, 01->Multibus memory,
 *		       10->page fault,   11->local RAM.
 *	PPP = protection, cccccccc = context
 *
 * protection codes are:	rwx supervisor, rwx user:
 *   0	------  No access
 *   1	r-x---
 *   2	rwx---
 *   3	rwx--x
 *   4	rwxr-x
 *   5	rwxrwx
 *   6	------	reserved for future use
 *   7	------	like maybe the context doesn't need to match?
 */
#define PROT_USED	USED	/* set to 1 on access */
#define PROT_DIRTY	DIRTY	/* set to 1 on writes */
/*#define PROT_MULTIBUS	0x2000	/* 1 -> multibus, 0 -> local */
/*#define PROT_MEMORY	0x1000	/* 1 -> multi mem, 0 -> multi I/O */
/*#define PROT_PRESENT	0x0800	/* fault if not set and accessed */
#define PROT_MBMEM	0x0000	/* multibus memory */
#define PROT_LOCMEM	0x2000	/* local memory */
#define PROT_MBIO	0x1000	/* multibus I/O */
#define	PROT_NOACCESS	(short)prot(0)
#define	PROT_R_X___	(short)prot(1)	/* Like ls -l modes */
#define	PROT_RWX___	(short)prot(2)
#define	PROT_RWX__X	(short)prot(3)
#define	PROT_RWXR_X	(short)prot(4)
#define	PROT_RWXRWX	(short)prot(5)
#define PROTMR_X___	(short)prot(6)
#define PROTMRWX___	(short)prot(7)

/* EXCEPTION reg read: fault bits, 0->true */
#define EXCEPTION_REG	(*EXCEPTION)
#define FAULT_PRESENT	PRESENT	/* Page not present */
#define FAULT_MAP	MAPERR	/* Some protection fault */
#define FAULT_TIMEOUT	TIMEOUT	/* Timeout */
#define FAULT_PARITY	PARERR	/* Parity error */

/* STATUS_REG write: cleared at power-on, HIGH BYTE is display reg/leds */
#define ENABLE_MBINT	INT_EN	/* 1 enables multibus int IPL5 */
#define ENABLE_PARITY	PAR_EN	/* 1 enables parity faults DONT SET!! */
#define ENABLE_MBUS	BINIT	/* active low - toggle this in software */
#define ENABLE_BOOT	NOTBOOT /* active low - set to 1 after booting */

/* Multibus: pmII occupies 2 meg of address space, set by dip on board.
 * Slightly less than one meg of this is "real" memory (lower meg),
 * the upper meg consisting of a page map for converting this virtual 
 * multibus address into a physical address.  The process is as follows:
 *
 *	A multibus address of X is seen by the board.  If X is within
 * 	the 2M window selected by the pm2 dipswitch, the address is 
 *	recognized.  If it is in the lower 240x4K of the window, the
 *	address is seperated into an offset (the low-order 12 bits),
 *	and a page (bits 12-19).  This page is used to index into the
 *	page map in the upper M of memory.  If the page is less than 240,
 *	the 12-bit page address is taken from the map (which occupies the
 *	upper M of address space.)  The page address and offset select
 *	the physical page.  If the page is >=240, the memory reference is
 *	treated as a mailbox interrupt, and the interrupt is triggered
 *	if enabled.
 *
 *	If the address is into the upper M of address space, the access
 *	is for the page table.  The request is not acknowledged if the
 *	access is disallowed by the EN bits in SR, or if the page referenced
 *	is >= 240.
 */

/* mouse goodies: */
#define BUTTONS		(*MOUSE_BUT&7)
/* for mouse quadrature */
#define MX_FIRE	XFIRE	/* x changed */
#define MX_UP	XDIR	/* if changed then up else down */
#define MY_FIRE	YFIRE	/* y changed */
#define MY_UP	YDIR	/* if changed then up else down */

/* Macro to poke first DUART to start refresh */
#define	refresh()\
	*(char *)0xfc4008 = 0xEB;\
	*(char *)0xfc400c = zero;\
	*(char *)0xfc400e = 28;\
	*(char *)0xfc401a = 0xF4

#define	HALF	0x80		/* Pages per 1/2 Meg */
#define ONEMEG 	0x100

