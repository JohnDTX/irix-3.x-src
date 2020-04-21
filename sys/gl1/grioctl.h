/*
 * Graphics ioctl's:
 *	- user issues a "grioctl(cmd, argvector)" system call
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/grioctl.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:23 $
 */

/* generic commands */
#define	GR_ALLOC	0		/* allocate graphics */
#define	GR_INIT		1		/* initialize graphics */
#define	GR_RESET	2		/* reset ge's */
#define	GR_QTEST	3		/* see if queue has any data */
#define	GR_QENTER	4		/* enter something in the queue */
#define	GR_QRESET	5		/* reset the queue's */
#define	GR_GETKEY	6		/* get a key from the keyboard */
#define	GR_ATTACHCURSOR	7		/* attach cursor to some valuators */
#define	GR_RESETCURSOR	8		/* reset cursor to valuators values */
#define	GR_QREAD	9		/* read element from queue */
#define	GR_GETKBD	10		/* get keyboard state info */
#define	GR_SETKBD	11		/* set keyboard state info */
#define	GR_MAPCOLOR	12		/* map a color register */
#define	GR_FREE		13		/* free graphics */

/* window stuff */
#define	GR_WATTACH	14		/* attach keyboard to a window */
#define	GR_GETWIN	15		/* get next free window # */
#define	GR_PUTWIN	16		/* release window */
#define	GR_SETPIECE	17		/* define window pieces */
#define	GR_GETPIECE	18		/* read piece list */
#define	GR_ENABLEWIN	19		/* enable window */
#define	GR_DISABLEWIN	20		/* disable window */
#define	GR_FLUSH	21		/* flush textports to screen */
#define	GR_BORDERON	22		/* enable border drawing */
#define	GR_BORDEROFF	23		/* disable border drawing */
#define	GR_SETPIECE1	24		/* like GR_SETPIECE, but don't redraw */

/* garbage */
#define	GR_TPBLANK	31		/* textport off (forever) */
#define	GR_TEXTCOLOR	32		/* set text color */
#define	GR_PAGECOLOR	33		/* set page color */
#define	GR_RINGBELL	34		/* ring the dinger */

/* more stuff */
#define	GR_SETVALUATOR	50		/* reset dial values */
#define	GR_DIALINIT	51		/* init dial box */
#define	GR_DIALTEXT	52		/* text for dial box */
#define	GR_DIALLEDS	53		/* turn off/on leds on dial box */

#define GR_CURBASE	55		/* pair cursor numbers w/ base addrs */
#define GR_CURCYCLE	56		/* how many cursors to cycle thru */
#define GR_CURFREQ	57		/* how often to cycle the cursors */
#define GR_ADDBLINK	58		/* add index to be blinked */
#define GR_DELBLINK	59		/* del index to be blinked */
#define GR_GETCHARINFO		80	/* get def height, width, descender */
#define GR_SETCHARINFO		81	/* set def height, width, descender */

#define GR_GETCHAROFFSETS	82	/* get offset table for def font */
#define GR_SETCHAROFFSETS	83	/* set offset table for def font */

/* structure for GR_MAPCOLOR */
struct	grmapcolor {
	short	gr_index;		/* index in color map (0-255) */
	u_char	gr_red;			/* red value */
	u_char	gr_green;		/* green value */
	u_char	gr_blue;		/* blue value */
};

/* structure for GR_ADD/DELBLINK */
struct	grblinkcolor {
	short	gr_index;		/* index in color map (0-255)
						(-1 if empty) */
	short	gr_brate;		/* rate at wich to blink this index */
	short	gr_bticks;		/* retraces left before blinking */
	char	gr_currently;		/* which color is current 0 or 1 */
	u_char	gr_red0;		/* original red value */
	u_char	gr_green0;		/* original green value */
	u_char	gr_blue0;		/* original blue value */
	u_char	gr_red1;		/* alternate red value */
	u_char	gr_green1;		/* alternate green value */
	u_char	gr_blue1;		/* alternate blue value */
};

/* structure to lump cursor number with bits */
struct	grcursorbase {
	short num;			/* cursor number */
	short base;			/* base address for this cursor */
	short gr_cconf;			/* cursor configuration */
};

/* a piece of a window */
struct	grpiece {
	short	gr_xmin, gr_xmax;	/* x location of piece */
	short	gr_ymin, gr_ymax;	/* y location of piece */
	short	gr_ly, gr_uy;		/* lower upper y bounds */
};

/* header for GR_SETPIECE info (followed by gr_pieces # of grpiece structs) */
struct	grpiecehdr {
	ushort	gr_win;			/* window # */
	ushort	gr_pieces;		/* # of pieces */
	ushort	gr_llx, gr_lly;		/* lower left x,y */
	ushort	gr_xlen, gr_ylen;	/* extents, in chars */
};

/* structure for GR_SETVALUATOR */
struct	grsetvaluator {
	short	gr_valuator;
	short	gr_initialvalue;
	short	gr_minvalue;
	short	gr_maxvalue;
};
