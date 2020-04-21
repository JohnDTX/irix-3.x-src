#define	SINGLEBUFFER			/* how to do display */
#define	NTEXTPORT	8		/* max # of active windows */
#define	MAXPIECES	25		/* max # of pieces per window */

/* parameters for the screen data structure */
#define	MAXROWS		40
#define	MAXCOLS		80
#define	MINROWS		2
#define	MINCOLS		10

/* font size for each dimension in pixels*/
#define	XLEN(l)		((l) * WIDTH)
#define	YLEN(l)		((l) * HEIGHT)
#define	XBORDER		4
#define	YBORDER		4

/*
 * Each line of text on the screen has a data structure which is used
 * to maintain the state of the line
 */
struct	row {
	u_char	*r_data;		/* pointer to line data */
	u_char	*r_color;		/* pointer to color data */
	short	r_maxcol;		/* maximum column in r_data */
	char	r_changed;		/* this line has changed */
	char	r_video;		/* has video attributes set */
	short	r_screeny;		/* location on screen of this line */
	u_char	r_buf[MAXCOLS + 1];	/* data for line (+1 for null) */
	u_char	r_colbuf[MAXCOLS];	/* data for colors */
};

/*
 * When a window is obscured, it is broken up into pieces, which
 * are described by the structure below
 */
struct	piece {
	short	p_state;		/* non-zero if in use */
	short	p_xmin, p_xmax;		/* physical screen locations */
	short	p_ymin, p_ymax;
	short	p_ly, p_uy;		/* lower and upper screen y */
};

/* each window on the screen is managed by the following structure */
struct	window {
	ushort	w_llx, w_lly;		/* lower left x,y */
	ushort	w_xlen, w_ylen;		/* extents, in characters */
	struct	piece w_piece[MAXPIECES];
	struct	piece w_sentinel;
};

/* each textport contains the following info */
struct	textport {
	struct	window tx_w;		/* window info */
	short	tx_state;		/* textport state */
	u_char	tx_foreground;		/* foreground color */
	u_char	tx_background;		/* background color */
	u_char	tx_curcolor;		/* current text color */
	u_char	tx_cursorcolor;		/* color of the cursor */
	u_char	tx_reverse;		/* reverse video color */
	u_char	tx_borderon;		/* border color (lit portion) */
	u_char	tx_borderoff;		/* border color (unlit portion) */
	short	tx_row, tx_col;		/* current cursor row and column*/
	short	tx_oldrow, tx_oldcol;	/* previous cursor row and column */
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

#ifdef	KERNEL
struct	textport txport[NTEXTPORT];
extern ushort HEIGHT;
extern ushort WIDTH;
extern ushort DESCENDER;
#endif
