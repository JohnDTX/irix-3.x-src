#ifndef SHMEMDEF
#define SHMEMDEF
/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include "gl.h"
#include "gltypes.h"
#include "device.h"
#include "addrs.h"

#define	WNTIMEOUT	(10*60)		/* # of seconds before screen blanks */
#define FONTRAM_STARTSIZE	(1*256)	/* must be multiple of (pattern size) */

/*
 * This version string is copied into the users shared memory smallbuf
 * after the program calls grioctl(GR_GRINIT).  It is used to verify
 * that the gl matches the running operating system.
 */
#define	GLVERSION	"GL2.4"
#define MAPTABSIZE	64

/*
 * "Shared" memory data structure for cooperative manipulation of the
 * graphics hardware, between the kernel and the user
 */
struct	shmem {
	short	EOFpending;		/* flag for waiting until interrupt */
	short	*intbuf;		/* pointer to current feedback buf */
	short	fastfeed;		/* fast feedback in progress (whoa) */
	long	intbuflen;		/* size of feedback buffer */
	short	numhits;		/* number of hits */
	short	hitbits;		/* hit bits from fbc */
	short	locked;			/* don't interrupt this gl program */
	unsigned short	inputchan;      /* the input channel of this process*/
	unsigned short	gfnum;	       /* the current gfport of this process*/
	windowstate ws;			/* current window state */
	short	ge_mask;		/* mask of ge chips found << 1 */
	short	ge_found;		/* # of ge chips found		*/
	short	qtop;			/* top element on queue		*/
	short	qdev;			/* device # for qread()		*/
	short	qvalue;			/* return value for qread()	*/
	char	isblanked;		/* screen is blanked */
	short	smallbuf[128];		/* for quicker feedback */
	short	inp, outp;		/* for nice mapcolor */
	short	indices[MAPTABSIZE];	/* array of indices */
	unsigned char rs[MAPTABSIZE];	/* array of rs */
	unsigned char gs[MAPTABSIZE];	/* array of gs */
	unsigned char bs[MAPTABSIZE];	/* array of bs */
};

/* bits in EOFpending */
#define EOFPENDINGBITS	0x3fff
#define VERTPENDINGBIT	0x4000

#ifdef	KERNEL /* per system structures */

/* the shmem struct stuff */
#ifdef V
struct shmem 	dummyshmem0;		/* kernel's shmem struct */
#endif
struct shmem    *gl_kernelshmemptr;	/* pointer to kernel's shmem struct */
struct shmem 	*gl_shmemptr;		/* current shmem ptr */

/* info about the hardware */
short		gefound;		/* # of ge chips found in gefind */
short		gemask;			/* mask of ge chips found << 1 */
unsigned long	gl_gestatus;		/* status of ge */
short		gl_fbcstatus;		/* status of fbc */
short		gl_fbcversion;		/* fbc microcode version */
short		gl_softkeyboard;	/* true if we have soft kbd */
short		havelpen;		/* do we have a light pen? */

/* hardware allocation globals */
short 		gl_textportno;		/* current keyboard textport */
struct inputchan *gl_curric;		/* current input chan */
struct inputchan *gl_gfport;		/* current gf port */
struct inputchan *gl_wmport;		/* current window manager  port */
struct inputchan *gl_kbport;		/* current keyboard manager port */
struct inputchan *gl_simport;		/* current kgl simulator port */
short		gl_wmbutton;		/* button for window manager */
short		gl_wmswapanytime;	/* if true, don't wait for wman */

/* device stuff */
buttondata 	gl_buttons[BUTCOUNT];	/* actual button state */
valuatordata	gl_valuators[VALCOUNT]; /* actual valuator positions */
retrevent	gl_retrevents[MAX_RETRACE_EVENTS]; /* blink stuff */
short 		gl_cursorxvaluator;	/* valuator for cursor x */
short 		gl_cursoryvaluator;	/* valuator for cursor y */
short		gl_cursorx;		/* actual cursor x */
short		gl_cursory;		/* actual cursor y */
short		gl_cursorxorgin;	/* current cursor glyph orgin x */
short		gl_cursoryorgin;	/* current cursor glyph orgin y */
short		gl_cursordrawn;		/* true if cursor is drawn */
short		gl_autocursor;		/* curson/cursoff */

/* miscellaneous stuff */
short		gl_ioerror;		/* error code from grioctl */
short		gl_vpoffsetx;		/* offset for cursor x position */
short		gl_vpoffsety;		/* offser for cursor y position */
long		gl_kwritemask;		/* bit planes available to win[0] */
long		gl_userwritemask;	/* bit planes available to users */
long		gl_kdbwritemask;	/* bit planes available to win[0] */
long		gl_userdbwritemask;	/* bit planes available to users */
short		gl_SwapCount;		/*  ??? */
short		gl_MaxSwapInterval;	/*  ??? */
long		gl_cfr;			/* CHANGE? */
short		gl_dcr;			/*  ??? */
short		gl_displayab;		/*  ??? */
short		gl_blankmode;		/* user has done a blankscreen() */
short		gl_isblanked;		/* kernel has done a kblankscreen() */
unsigned long	gl_framecount;		/* count frames */
unsigned long	gl_lastupdate; 		/* time last wn_redisplay was done */
short		gl_kbdstate;		/* state bits for keyboard */
short		gl_numdoublebufferers;	/* number of db'ed graphics procs */
short		gl_numrgbs;		/* number of rgb'ed graphics procs */
long		gl_cursorconfig;
long		gl_cursorcolor;
long		gl_rcursorcolor;
long		gl_gcursorcolor;
long		gl_bcursorcolor;
long		gl_cursorwenable;
long		gl_rcursorwenable;
long		gl_gcursorwenable;
long		gl_bcursorwenable;
long		gl_cursoraddr;
short		gl_needtodoswaps;
short		gl_didswap;
struct piece	*gl_piecefreelist;	/* free list of pieces */
short		gl_charwidth;		/* width of default font */
short		gl_charheight;		/* height of default font */
short		gl_chardescender;	/* descender of default font */
short		gl_dialport;		/* serial port for dial box */
short		gl_bpadport;		/* serial port for bit pad */

/* stuff for feedback */
short		*gl_origfbaddr;		/* orig fb addr */
long		gl_origfbcount;		/* orig fb count */
short		*gl_fbaddr;		/* current fb addr */
long		gl_fbcount;		/* current fb count */
struct inputchan *gl_fbwn;		/* window doing feedback */
short		gl_lock;		/* graphics locked up for feedback */

struct inputchan *getic();
struct gfport *getgf();

#endif KERNEL

#endif SHMEMDEF
