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

typedef	struct {
	int	xorg, yorg;
	int	xlen, ylen;
} rect_t;

/* parameters for the screen data structure */
#define	MAXROWS		66		/* maximum sized textport, in chars */
#define	MAXCOLS		132
#define	MINROWS		2		/* minimum sized textport, in chars */
#define	MINCOLS		10

/* font size for each dimension in pixels*/
#define	HEIGHT		15		/* height of char in pixels */
#define	WIDTH		9		/* width of char in pixels */
#define	DESCENDER	2		/* descender length */
#define	XLEN(n)		((n) * font_width)	/* length in pix of n chars */
#define	YLEN(n)		((n) * font_height)	/* height in pix of n chars */

/*
 * NROWS is computed by dividing the size of the micro-code data area
 * (32768 bytes) by the amount of data in a row (80 bytes).  We round
 * this down to 40 line chunks, thus producing a maximum of 10 textports.
 */
#define	NROWS		400
#define	NINCHANS	20
#define	NGFPORTS	20
#define	NTXPORTS	10		/* don't change this! */
#define	NPORTS		(NGFPORTS + NTXPORTS)
#define	NPIECES		(8 * NPORTS)	/* might be big enough */

/*
 * Each line of text on the screen has a data structure which is used
 * to maintain the state of the line
 */
struct	row {
	char	*r_data;		/* pointer to line data */
	short	r_screeny;		/* location on screen of this line */
	short	r_maxcol;		/* maximum column in r_data */
	char	r_changed;		/* this line has changed */
	char	r_video;		/* has video attributes set */
};

/* each textport contains the following info */
struct	txport {
	short	tx_state;		/* textport state */
	short	tx_no;			/* textport number */
	rect_t	tx_r;			/* location in window */
	struct	piece *tx_piecelist;	/* list of pieces */
	/*
	 * Line information
	 */
	int	tx_lines;		/* # of lines */
	int	tx_firstline;		/* first line displayed */
	int	tx_lastline;		/* last line displayed */

	short	tx_cols, tx_rows;	/* extents, in characters */
	short	tx_row, tx_col;		/* current cursor row and column*/
	short	tx_oldrow, tx_oldcol;	/* previous cursor row and column */
	char	tx_keypadmode;		/* texport is in keypad mode */
	long	tx_textcolor;		/* foreground color */
	long	tx_reversecolor;	/* reverse video color */
	long	tx_textwritemask;	/* foreground writemask */
	long	tx_pagecolor;		/* background color */
	long	tx_pagewritemask;	/* background writemask */
	long	tx_curcolor;		/* current text color */
	long	tx_cursorcolor;		/* color of the cursor */
	short	tx_cursor_r,
		tx_cursor_g,
		tx_cursor_b;		/* rgb value of cursor color */
	short	tx_cmd;			/* read command in progress */
	short	tx_count;		/* read count */
	char	tx_readbuf[MAXCOLS+1];	/* read buffer (+1 for a null) */
	struct	row tx_display[MAXROWS];/* display data */
};

/* textport tx_state's */
#define	TX_READ0	0x0001		/* reading args for \E0x command */
#define	TX_READ9	0x0002		/* reading args for \E9x command */
#define	TX_READY	0x0003		/* reading args for cursor move */
#define	TX_READ7	0x0004		/* reading args for \E7x command */
#define	TX_READTITLE	0x0005		/* reading title string */
#define	TX_READCOMMA	0x0006		/* reading a screen size command */
#define	TX_READBINDING	0x0007		/* read a function key binding */
#define	TX_READKEY	0x0008		/* read key to output binding for */
#define	TX_TYPE		0x000F		/* type mask for above read's */

#define	TX_NORMAL	0x0000		/* normal display mode */
#define	TX_COLLECT	0x0010		/* collect args for a command */
#define	TX_ESCAPE	0x0020		/* just recieved an escape */
#define	TX_STATEBITS	0x00F0		/* state mask for above bits */

#define	TX_BLINKING	0x0100		/* textport cursor blinks */
#define	TX_GOING	0x0200		/* textport is not stopped */
#define	TX_OPEN		0x0400		/* textport is open and in use */
#define	TX_BUSY		0x0800		/* busy redisplaying it */
#define	TX_SELECTED	0x1000		/* textport is selected */
#define	TX_ON		0x2000		/* text port is displayable */
#define	TX_REDISPLAY	0x4000		/* text needs to be redrawn */
#define	TX_SCROLLED	0x8000		/* text has scrolled */

struct	txport txport[1];

#endif WINDOWDEF
