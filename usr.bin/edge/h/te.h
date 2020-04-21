#ifndef	TE_SCROLLED

/*
 * Terminal emulator stuff.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/h/RCS/te.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:06 $
 */

/* XXX RGB mode? */

typedef struct {
	long	te_flags;		/* state of termulator */
	long	te_mode;		/* mode of termulator */
	textview *te_tv;		/* pointer to view */
	textframe *te_tf;		/* pointer to frame */
	/*
	 * Row and column of cursor position.  The te_row is relative
	 * to the top row displayed in the view (tv_toprow).
	 */
	short	te_row, te_col;
	short	te_toprow;		/* top displayed row */
	short	te_rows, te_cols;	/* maximums */
	short	te_maxrows;		/* total rows to keep in frame */
	short	te_cursorcolor;		/* cursor color to use */
	int	te_fgcolor;		/* fg color to use */
	int	te_bgcolor;		/* bg color to use */
	int	te_reversecolor;	/* reverse video color to use */
	char	te_save[200];		/* escape stuff */
	int	te_savecount;		/* # of chars in save */
	int	(*te_putchars)();	/* putchars function */
} termulator;

/* te_flags's */
#define	TE_SCROLLED	0x00000001	/* scrolled within view */
#define	TE_REPAINT	0x00000002	/* next display needs a full repaint */
#define	TE_STOPPED	0x00000004	/* hold display */
#define	TE_BLINK	0x00000008	/* blink cursor */
#define	TE_CURSOR	0x00000010	/* non-zero if cursor is displayed */

/* interface */
extern	termulator	*tenew();	/* make a new emulator */
extern	textframe	*teframe();	/* return text frame being used */

/* internal interface */
extern	int		termulate_iris();
extern	void		temoveto();	/* move output cursor */
extern	void		tecleartoeof();	/* clear to end of frame */
extern	void		tecleartoeol();	/* clear to end of line */
extern	void		teinsertln();	/* insert line */
extern	void		tedeleteln();	/* delete line */

#endif	/* TE_SCROLLED */
