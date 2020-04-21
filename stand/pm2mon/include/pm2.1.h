/*
 * PMII addresses and bits
 *	all addresses A also appear at A-0xf80000 if boot state is set
 */

/* interrupt levels 
 *	7 - mouse quadrature
 *	6 - both duarts
 *	5 - Multibus level 5
 *	4 - mouse buttons
 *	    mailbox
 *	    ppoutready
 *	    ppinready
 */

/* ROM base addresses - 64K apart */
#define ROM0		((short *) 0xF80000)
#define ROM1		((short *) 0xF90000)
#define ROM2		((short *) 0xFA0000)
#define ROM3		((short *) 0xFB0000)

/* ROM size, actual and virtual */
#define ROMSIZE		(1<<15)
#define ROMSPACING	(1<<16)

/*
 * page and protection maps
 *	ram and multibus accesses look up mapping and protection bits
 *	for address A by computing
 *		(context<<4) | (A>>12)
 *	addresses that map into the same map address are resolved by
 *		comparing current context with the context stored
 *		in the protection map for equality
 */
#define PAGEMAP		((short *) 0xfc0000)	/* high 4 bits MUST be zero */
#define PROTMAP		((short *) 0xfc2000)
#define context(x)	(x&0xff)	/* stored context */
#define prot(x)		((x&0xf)<<8)	/* stored protection code */
/* possible protection codes are
	"-------",	/* no access
	"-r-x---",	/* supervisor read, execute ...
	"-rwx---",	/* ... and write
	"-rwx--x",	/* user execute ...
	"-rwxr-x",	/* ... read ...
	"-rwxrwx",	/* ... and write
	"mr-x---",	/* context need not match
	"mrwx---",	/* Dangerous!
 */
#define RAM		0x2000	/* 1->local, 0->Multibus */
#define MEM		0x1000	/* 01->Multibus I/O, 00->Multibus memory */
				/* 11->page fault, 10->local RAM */
#define DIRTY		0x4000	/* page has been written */
#define USED		0x8000	/* page has been touched */

#define UART0		((char *) 0xfc4000)	/* uses high (even) byte */
#define UART1		((char *) 0xfc6000)	/* both int at level 6 */
#define CONTEXT		((char *) 0xfc8001)	/* context register */

/* the status register is cleared upon reset. All bits ar
   read/write and active low unless indicated */
#define STATUS		((short *) 0xfc9000)	/* read/write */
#define DIAG(x)		(0xf&x)	/* low four bits */
#define INT_EN		0x10	/* enable mailbox interrupt. Active high.*/
				/* disable-enable to clear */
#define PAR_EN		0x20	/* enable parity detection. Active high. */
#define BINIT		0x40	/* Multibus init. Pull high for init.*/
#define NOTBOOT		0x80	/* 0 for boot state. Pull high to clear.*/
/* note - EN0/EN1 is supposed to govern multibus MAP access, but it currently
   governs multibus MEMORY access */
#define EN0		0x100	/* 0 enables external multibus memory access */
#define EN1		0x200	/* 0 enables external multibus memory
				   write access */
#define USERGE		0x400	/* allow user mode GE access. Active high.*/
#define USERPP		0x800	/* allow user mode PP access. Active high.*/

/* 0 bits mean that level 4 interrupt is active. */
/* All bits in the exception register are active low.*/
#define EXCEPTION	((short *) 0xfca000)	/* read only */
#define PRESENT		0x1	/* 1 if present. 0 if page fault */
#define MAPERR		0x2	/* 0 if error */
#define TIMEOUT		0x4	/* 0 if error */
#define PARERR		0x8	/* 0 if error */
#define MBINT		0x10	/* mouse button interrupt */
#define EXTINT		0x20	/* mailbox interrupt */
#define P0INT		0x40	/* PP receive interrupt */
#define P1INT		0x80	/* PP transmit interrupt */

#define MOUSE		((short *) 0xfcc000)	/* mouse quadrature */
#define XFIRE		0x1	/* x position has changed */
#define XDIR		0x2	/* if so, direction of change */
#define YFIRE		0x4	/* ditto for y */
#define YDIR		0x8

#define MBUT		((short *) 0xfce000)	/* read buttons (supervisor) */
#define RIGHT_BUT	0x1		/* touching this location also */
#define MIDDLE_BUT	0x2		/* clears the button interrupt */
#define LEFT_BUT	0x4

/* config register - switch 1 is LSB, 'open' = set, only eight switches read*/
#define CONFIG		((char *) 0xfd0000)	/* 8 bit dipswitch */
#define MBREAD		((short *) 0xfd2000)	/* read buttons (user mode) */
#define GEport		((short *) 0xfd4000)	/* what else? */
#define PPport		((short *) 0xfd6000)	/* parallel port */
/*
#define GEPORT		GEport	/* for the fbc stuff */
