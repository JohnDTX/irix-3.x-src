/*
 * 	common.h - layout of the global communication area
 *
 *	   the global communication area resides on-board at
 *	   0x200 - 0x230.  Its layout is defined in this file.
 *
 */
# define BOOTSTRSIZE	0x60
typedef struct
{
	/* 0x200 (0x20) */
	char memfound[32];	/* one byte per physical half-meg */

	/* 0x220 (0x04) */
	unsigned long membits;	/* compressed form of above;
				   membits[0:31] <- memfound[0..31] */

	/* 0x224 (0x02) */	/* mbmem parameters.  external
				   processors can write to our memory
				   location page (mbphys) at multibus
				   location (mbphys) via multibus page
				   (mblog).  nmbpages gives the number
				   of pages so mapped.
				   usually	mblog == mbw + 0x10
				   for slaves	mbphys = 0
				   for masters	mbphys = mblog */
	short mblog;		/* lowest mbmem page mapped to physical mem */

	/* 0x226 (0x02) */
	short mbw;		/* our page address on the multibus */

	/* 0x228 (0x02) */
	short flags;

	/* 0x22A (0x02) */
	unsigned short config;	/* configuration switch settings - 
				   read at each reset */

	/* 0x22C (0x02) */
	unsigned short reboot;

	/* 0x22E (0x02) */
	unsigned short boottype;

	/* 0x230 (0x02) */	/* screen management data	   XXX */
	unsigned char screenx,screeny;	/* current cursor position XXX */

	/* 0x232 (0x02) */
	unsigned char nblines, savenblines; /* #lines to clear after cursor */

	/* 0x234 (0x04)*/
	unsigned long checkval;

	/* 0x238 (0x02) */
	unsigned char imr,pad;

	/* 0x23A (0x04) */
	unsigned short mbphys, nmbpages;

	/* 0x23E (0x01) */	/* the dc4 configuration
				   for low and high modes.  Bits 0-3 of
	   			   this byte are bits d12-d15 of the dcreg
				   for LOW mode.  Bits 4-7 of this byte are
				   bits d12-d15 of the dcreg for HIGH mode.
				   This byte is only valid if the keyboard
				   is MICSW */
	unsigned char dcconfig;

	/* 0x23F (0x01) */
	unsigned char addr488;	/* 488 addr used for booting */

	/* 0x240 (0x60) */
	char bootstr[BOOTSTRSIZE];

}	_comm_t;

extern char common_area[];
# define _commdat	((_comm_t *)common_area)
# define common_size	0x100
# define _commend	(common_area+common_size)

/* .reboot values */
# define MAGIC_REBOOT_VALUE	0xF8C4
# define MAGIC_NETBOOT		1
# define MAGIC_DBOOT		0

/* .flags bits */
# define MASTER			0x8000
# define TERMULATE		0x4000
# define BREAK			0x2000
# define BREAK_IN_PROGRESS	0x1000
# define MICROSW		0x0800
# define USER_PROGRAM		0x0400
# define DC_HIGH		0x0200
# define ADDR488_VALID		0x0100
# define BREAKPENDING		(BREAK|BREAK_IN_PROGRESS)
# define GL1_PRESENT		0x0080
# define GL2_PRESENT		0x0040

/* .flag macros */
# define TERMULATING	(_commdat->flags & TERMULATE)
# define DOINGBREAK	(_commdat->flags & BREAK_IN_PROGRESS)
# define ISMASTER	(_commdat->flags & MASTER)
# define SETTERMULATING	(_commdat->flags |= TERMULATE)
# define RESETTERMULATING (_commdat->flags &= ~TERMULATE)

# define BREAKON	(_commdat->flags & BREAK)
# define SETBREAK	(_commdat->flags |= BREAK)
# define RESETBREAK	(_commdat->flags &= ~BREAK)
# define TOGGLEBREAK	(_commdat->flags ^= BREAK)
# define screen_x	(_commdat->screenx)		/* XXX */
# define screen_y	(_commdat->screeny)		/* XXX */
# define CHECKVAL	0x7efe2501
# define ISCOMMONOK	(_commdat->checkval == CHECKVAL)
# define SETASCII (_commdat->checkval=CHECKVAL,_commdat->flags&=~MICROSW)
# define SETMICSW (_commdat->checkval=CHECKVAL,_commdat->flags|=MICROSW)
# define ISMICROSW		(_commdat->flags & MICROSW)
# define ISASCII	(!ISMICROSW)
# define RUNNING_USER_PROGRAM	(_commdat->flags & USER_PROGRAM)
# define SET_USER_PROGRAM	(_commdat->flags |= USER_PROGRAM)
# define CLEAR_USER_PROGRAM	(_commdat->flags &= ~USER_PROGRAM)
# define DEFAULT_DC_HIGH	(_commdat->flags & DC_HIGH)
# define ISADDR488_VALID	(_commdat->flags & ADDR488_VALID)
# define SETGL1		(_commdat->flags |= GL1_PRESENT)
# define SETGL2		(_commdat->flags |= GL2_PRESENT)
# define ISGL1		(_commdat->flags & GL1_PRESENT)
# define ISGL2		(_commdat->flags & GL2_PRESENT)
# define CLEARGL	(_commdat->flags &= ~(GL1_PRESENT|GL2_PRESENT))
