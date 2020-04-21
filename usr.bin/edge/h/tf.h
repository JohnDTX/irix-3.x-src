#include "gl.h"
#ifndef	__TF_H__
#define	__TF_H__
/*
 * Text frame defines
 */

/*
 * Looks for each character in a text line.  Number of color handles
 * defined is known below in the textview data structure.  Number of
 * font handles is well known below in the textview data structure.
 * A couple of state bits are kept in the looks, to reduce overhead.
 * LOOKS_SAME is set non-zero (only in the textline data structure) if
 * all the characters on a given line share the same looks information.
 */
#define	LOOKS_SAME	0x80000000	/* see textline definition */
#define	LOOKS_FONT	0x7F000000	/* font handle */
#define	LOOKS_FONTSHIFT	24
#define	LOOKS_FG	0x003F0000	/* foreground color handle */
#define	LOOKS_FGSHIFT	16
#define	LOOKS_BG	0x00003F00	/* background color handle */
#define	LOOKS_BGSHIFT	8
#define	LOOKS_SELECT	0x00008000	/* text is selected */
#define	LOOKS_UNDER	0x00004000	/* text is underlined? */
#define	LOOKS_INDEX	0x000000FF	/* bits where index goes to */
#define	LOOKS_MASK	0x7F3FFF00	/* bits for looks comparisons */

/*
 * Return flags for tv_pixtopos and tv_postopix, indicating various
 * out-of-bounds conditions encountered in mapping pixels to
 * text positions, and vice versa.
 */
#define	IN_BOUNDS	0x00	/* pixel, position lie in textview and frame */
#define	OUTOF_TF	0x0F	/* text position falls out of textframe bounds*/
#define	COL_OUTOF_TF	0x0C	/* col position falls out of textframe bounds */
#define	COL_BELOW_TF	0x08	/* col position falls below textframe bounds */
#define	COL_ABOVE_TF	0x04	/* col position falls above textframe bounds */
#define	ROW_OUTOF_TF	0x03	/* row position falls out of textframe bounds */
#define	ROW_BELOW_TF	0x02	/* row position falls below textframe bounds */
#define	ROW_ABOVE_TF	0x01	/* row position falls above textframe bounds */
#define	OUTOF_TV	0xF0	/* pixel falls out of textview bounds */
#define	X_OUTOF_TV	0xC0	/* x pixel falls out of textview bounds */
#define	X_BELOW_TV	0x80	/* x pixel falls below textview bounds */
#define	X_ABOVE_TV	0x40	/* y pixel falls above textview bounds */
#define	Y_OUTOF_TV	0x30	/* y pixel falls out of textview bounds */
#define	Y_BELOW_TV	0x20	/* y pixel falls below textview bounds */
#define	Y_ABOVE_TV	0x10	/* y pixel falls above textview bounds */

#ifdef	INLIB
/*
 * A text line.  Text lines which share a common looks for all the characters
 * in the line, will have the LOOKS_SAME bit set, and will use the
 * tl_chars pointer to address the text data.  Lines which don't have
 * common characters will use the tl_ldata pointer.
 */
typedef struct textline {
	struct textline	*tl_next, *tl_prev;
	long		tl_looks;	/* how line looks */
	short	 	tl_len;		/* # of chars in line */
	short		tl_space;	/* length of allocated data */
	short		tl_height;	/* maximum height of chars in line */
	short		tl_width;	/* width of line, in pixels */
	short		tl_dirty;	/* non-zero if line needs painting */
	union {
	    unsigned char *tlu_cdata;
	    long	*tlu_ldata;
	} tlu;
#define	tl_cdata	tlu.tlu_cdata
#define	tl_ldata	tlu.tlu_ldata
} textline;
#endif

/*
 * A text coordinate.  If tc_row < 0, then row is at end-of-frame.  If
 * tc_col < 0, then column is at end-of-line.
 */
typedef struct {
	long	tc_row;			/* row */
	long	tc_col;			/* column */
} textcoord;

#ifdef	INLIB
/*
 * A text frame.  A text frame is a dynamic array of pointers to textline's.
 * Also contained in the text frame are "point" and "mark" pointers, used
 * for some of the region operators.
 */
typedef struct {
	textline *tf_first;		/* pointer to first group */
	textline *tf_last;		/* pointer to last group */
	long	tf_rows;		/* total lines in frame */
	long	tf_looks;		/* current text frame looks */
	long	tf_writemask;		/* write mask for changelooks */
	textcoord tf_point;		/* point */
	textcoord tf_mark;		/* mark */
} textframe;

/*
 * A text view.  A text view is a displayable view into a text frame, showing
 * some portion of the text frame.
 */
#define	TV_COLOR_HANDLES	64
#define	TV_FONT_HANDLES		128
typedef struct {
	textframe *tv_tf;		/* pointer to text frame */
	struct {
		short	index;		/* index to use */
		short	r, g, b;	/* rgb value to use, if RGBmode() */
	} tv_h[TV_COLOR_HANDLES];
	unsigned short tv_bg;		/* view default background handle */
	unsigned short tv_selfg;	/* view selected foreground handle */
	unsigned short tv_selbg;	/* view selected background handle */
	struct textfont {
		short	valid;		/* non-zero means valid data */
		short	font;		/* font index to use */
		char	*filename;	/* file name of font */
		short	h, d;		/* height, descender */
		long	*widthtab;	/* table of character widths */
	} tv_fontinfo[TV_FONT_HANDLES];
	long	tv_toprow;		/* top displayable row */
	long	tv_xsize, tv_ysize;	/* size of display region */
	long	tv_flags;		/* random flag bits */
	long	tv_leftpix;		/* leftmost pixel to draw at */
	union {
		long	tvu_tabspace;	/* tab spacing, in pixels */
		struct {
			long	num;	/* number of tab stops */
			long	*spaces;/* pixel positions of stops */
		} tvu_setting;
	} tvu;
#define	tv_tabspace	tvu.tvu_tabspace
#define	tv_stops	tvu.tvu_setting.num
#define	tv_spacing	tvu.tvu_setting.spaces
} textview;

/* textview tv_flags */
#define	TV_VARIABLETABS	0x00000001	/* use tab stops info */
#define	TV_REDRAW	0x00000002	/* needs a full redraw */
#define	TV_SELECTTOEDGE	0x00000004	/* draw select hilite to edge of view */

/*
 * Internal point-mark op's
 */
#define	TFPMOP_GETASCII	0		/* get ascii data out */
#define	TFPMOP_GETTEXT	1		/* get text data out */
#define	TFPMOP_SELCOUNT	2		/* find size of selected region */
#define	TFPMOP_CHANGE	3		/* change looks */

#ifdef	DEBUG
/*
 * Debugging hooks
 */
extern	void		tfassert();
#define ASSERT(EX) if (EX) ; else tfassert("EX", __FILE__, __LINE__)
#else
#define	ASSERT(EX)
#endif

/*
 * If your libc bcopy can't cope with overlapping source/dest, then you
 * have to write one that does.
 */

/*
 * These are mostly for lint
 */
#define	OVBCOPY(f, t, c) \
	bcopy((char *) (f), (char *) (t), (long) (c))
#define	BCOPY(f, t, c) \
	bcopy((char *) (f), (char *) (t), (long) (c))
#define	MALLOC(type, len) \
	(type) malloc((unsigned) (len))
#define	REALLOC(type, cp, len) \
	(type) realloc((char *) (cp), (unsigned) (len))
#define	CALLOC(type, nelem, size) \
	(type) calloc((unsigned) (nelem), (unsigned) (size))
#define	FREE(p) \
	free((char *) (p))

extern	char *malloc();
extern	char *calloc();
extern	void bcopy();
extern	void free();

/* internal functions */
extern	textline	*tf_findline();
#else	/* INLIB */
typedef	char	*textframe;		/* sorry, you can't use it */
typedef	char	*textview;		/* sorry, you can't use it */
#endif

/* exported functions */
extern	textframe	*tfnew();
extern	textview	*tvnew();
extern	int		tfputtext();
extern	int		tfwrttext();
extern	void		tffree();

#endif	/* __TF_H__ */
