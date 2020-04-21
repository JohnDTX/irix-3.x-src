#ifndef WINDOWDEF
#define WINDOWDEF
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

/* parameters for the screen data structure */
#define	MAXROWS		40		/* maximum sized textport, in chars */
#define	MAXCOLS		80
#define	MINROWS		2		/* minimum sized textport, in chars */
#define	MINCOLS		10

/* font size for each dimension in pixels*/
#define	XLEN(n)		((n) * gl_charwidth)	/* length in pix of n chars */
#define	YLEN(n)		((n) * gl_charheight)	/* height in pix of n chars */
#define	XBORDER		4		/* this should be four (4) (IV) */
#define	YBORDER		4

/*
 * NROWS is computed by dividing the size of the micro-code data area
 * (32768 bytes) by the amount of data in a row (80 bytes).  We round
 * this down to 40 line chunks, thus producing a maximum of 10 textports.
 */
#define	NROWS		400
#ifdef UNIX
#   define	NINCHANS	20
#   define	NGFPORTS	20
#   define	NTXPORTS	10		/* don't change this! */
#endif
#ifdef V
#   define	NINCHANS	1
#   define	NGFPORTS	1
#   define	NTXPORTS	1
#endif
#define	NPORTS		(NGFPORTS + NTXPORTS)
#define	NPIECES		(8 * NPORTS)	/* might be big enough */

/*
 * When a window is obscured, it is broken up into pieces, which
 * are described by the structure below
 */
struct	piece {
	struct	piece *p_next;		/* link to next piece */
	short	p_xmin, p_xmax;		/* x range for piece */
	short	p_ymin, p_ymax;		/* y range for piece */
};

/* each window on the screen is managed by the following structure */
struct	gfport {
	struct gfport		*gf_next;		/* general link */
	short			gf_no;			/* struct number */
	struct piece		*gf_piecelist;		/* list of pieces */
	struct inputchan 	*gf_ic;		       /* ptr to input chan */
	short 			gf_llx, gf_lly;		/* lower left x,y */
	short			gf_urx, gf_ury;		/* lower left x,y */
};

/*
 * Each line of text on the screen has a data structure which is used
 * to maintain the state of the line
 */
struct	row {
	char	*r_data;		/* pointer to line data */
#ifdef	RV_8BIT
	char	*r_flags;		/* flag bytes for each row byte */
#endif
	short	r_screeny;		/* location on screen of this line */
	char	r_maxcol;		/* maximum column in r_data */
	char	r_changed;		/* this line has changed */
	char	r_video;		/* has video attributes set */
};

/* r_flags */
#define FLAG_REV	0x80		/* reverse video on */

/* each textport contains the following info */
struct	txport {
	short	tx_state;		/* textport state */
	short	tx_no;			/* textport number */
	struct	piece *tx_piecelist;	/* list of pieces */
	short	tx_llx, tx_lly;		/* lower left x,y of textport */
	char	tx_cols, tx_rows;	/* extents, in characters */
	char	tx_row, tx_col;		/* current cursor row and column*/
	char	tx_oldrow, tx_oldcol;	/* previous cursor row and column */
	char	tx_keypadmode;		/* texport is in keypad mode */
	long	tx_textcolor;		/* foreground color */
	long	tx_reversecolor;	/* reverse video color */
	long	tx_textwritemask;	/* foreground writemask */
	long	tx_pagecolor;		/* background color */
	long	tx_pagewritemask;	/* background writemask */
	long	tx_curcolor;		/* current text color */
	long	tx_cursorcolor;		/* color of the cursor */
	short	tx_cmd;			/* read command in progress */
	short	tx_count;		/* read count */
	char	tx_readbuf[2];		/* read buffer */
	struct	row tx_display[MAXROWS];/* display data */
};

/* textport tx_state's */
#define	TX_READ0	0x0001		/* reading args for \E0x command */
#define	TX_READ9	0x0002		/* reading args for \E9x command */
#define	TX_READY	0x0003		/* reading args for cursor move */
#define	TX_READ7	0x0004		/* reading args for \E7x command */
#define	TX_TYPE		0x000F		/* type mask for above read's */

#define	TX_NORMAL	0x0000		/* normal display mode */
#define	TX_COLLECT	0x0010		/* collect args for a command */
#define	TX_ESCAPE	0x0020		/* just recieved an escape */
#define	TX_STATEBITS	0x00F0		/* state mask for above bits */

#define	TX_DRAWBORDER	0x0200		/* draw border, if TX_BORDER is on */
#define	TX_OPEN		0x0400		/* textport is open and in use */
#define	TX_BUSY		0x0800		/* busy redisplaying it */
#define	TX_BORDER	0x1000		/* draw a border around the textport */
#define	TX_ON		0x2000		/* text port is displayable */
#define	TX_REDISPLAY	0x4000		/* text needs to be redrawn */
#define	TX_SCROLLED	0x8000		/* text has scrolled */

#ifdef KERNEL
/* XXX maybe we can move some of this into the shared memory? */
struct inputchan {
	struct shmem 		*ic_shmemptr;		/* ptr to shmem */
	struct txport		*ic_tx;			/* link to txport */
	short   		ic_qreadwait;		/* proc blocked */
	long  			ic_oshandle;		/* ptr to proc desc */
	short			ic_no;
	struct gfport		*ic_gf;
	queueentry 		ic_queue[BUFFER_SIZE + 1];
	queueentry 		*ic_queuein, *ic_queueout;
	short 			ic_SwapInterval;
	char			ic_keypadmode;
	short			ic_globalerrorcount;
	short			ic_errordevice;
	procbuttondata		ic_procbuttons[BUTCOUNT];
	procvaluatordata	ic_procvaluators[VALCOUNT];
	proctimerdata		ic_proctimers[TIMCOUNT];
	connection		*ic_outcon[OUTCOUNT];
	short			ic_consleep;
	long 			ic_doqueue;
	struct inputchan	*ic_next;		/* general link */
	short			ic_swapwaiting;		/* set if waiting */
	char			ic_displaymode;		/* cur display mode */
	char			ic_holding;		/* waiting for wman */
	short			ic_curoffsetx;		/* cursor offsets */
	short			ic_curoffsety;
	short			ic_sendwait;		/* waiting for reply */
	short			ic_bpadused;
	short			ic_dialused;
}; 

/* bits in ic_doqueue */
#define	DQ_KEYBOARD	1
#define	DQ_RAWKEYBOARD	2
#define	DQ_VALMARK	4
#define	DQ_ERRORS	8
#define	DQ_REDRAW	16 
#define	DQ_MODECHANGE	32
#define	DQ_INPUTCHANGE	64
#define	DQ_QFULL	128
#define	DQ_PIECECHANGE	256
#define	DQ_WINCLOSE	512

/* displaymode */
#define	MD_SINGLE	0
#define	MD_DOUBLE	1
#define	MD_RGB		2

#endif

#ifdef	KERNEL
struct	txport		txport[NTXPORTS];
struct	gfport		gfport[NGFPORTS];
struct  inputchan       inchan[NINCHANS];
struct	piece		piece[NPIECES];

#ifndef NULL
#define NULL	0
#endif NULL

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif MIN

struct	inputchan *gr_getgrhandle();
struct	piece *gr_allocpieces();
#endif KERNEL

#endif WINDOWDEF
