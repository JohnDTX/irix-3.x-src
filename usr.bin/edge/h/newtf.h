#ifndef	LOOKS_SAME
#include "pane.h"
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
 * A text line.  Text lines which share a common looks for all the characters
 * in the line, will have the LOOKS_SAME bit set, and will use the
 * tl_chars pointer to address the text data.  Lines which don't have
 * common characters will use the tl_ldata pointer.
 */
typedef struct {
	struct textline	*tl_next;	/* next line in list */
	struct textline	*tl_prev;	/* previous line in list */
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

/*
 * A text coordinate.  If tc_row < 0, then row is at end-of-frame.  If
 * tc_col < 0, then column is at end-of-line.
 */
typedef struct {
	long	tc_row;			/* row */
	long	tc_col;			/* column */
} textcoord;

/*
 * A text frame.  A text frame is a dynamic array of pointers to textline's.
 * Also contained in the text frame are "point" and "mark" pointers, used
 * for some of the region operators.
 */
typedef struct {
	textline *tf_first;		/* pointer to first line */
	textline *tf_last;		/* pointer to last line */
	textline *tf_toprow;		/* pointer to line at toprow */
	textline *tf_pointline;		/* pointer to line at point */
	textline *tf_markline;		/* pointer to line at mark */
	long	tf_rows;		/* total lines in frame */
	long	tf_looks;		/* current text frame looks */
	textcoord tf_point;		/* point */
	textcoord tf_mark;		/* mark */
	long	tf_maxcols;		/* maximum column number */
	long	tf_maxrows;		/* maximum row number */
	long	tf_flags;		/* random state flags */
	long	tf_writemask;		/* write mask for changelooks */
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
/* XXX fix me */
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

/*
 * Interface procedures.
 */
extern	textview	*tvnew();	/* allocate a text view */
extern	textframe	*tfnew();	/* allocate a text frame */
extern	void		tvfree();	/* delete a text view */
extern	void		tffree();	/* delete a text frame */
extern	textview	*tvclone();	/* clone a text view */
extern	textframe	*tfclone();	/* clone a text frame */
/* XXX put the rest in */
extern	void		tfdelete();	/* delete between point and mark */
extern	long		tfnumrows();	/* get number of rows in frame */
extern	void		tfgetmark();	/* get mark */
extern	void		tfsetmark();	/* set mark */
extern	void		tfgetpoint();	/* get point */
extern	void		tfsetpoint();	/* set point */
extern	int		tvsetstops();	/* set tab stops */
extern	long		tvgetstops();	/* get tab stops */
extern	void		tfchangelooks();
extern	long		tfgetwritemask();
extern	void		tfsetwritemask();
extern	int		tvpixtopos();

/* internal utilities */
extern	textgroup	*tgnew();	/* allocate a text group */
extern	textline	*tlnew();	/* allocate a text line */
extern	void		tgfree();	/* delete a text group */
extern	void		tlfree();	/* delete a text line */
extern	textgroup	*tgclone();	/* clone a text group */
extern	textline	*tlclone();	/* clone a text line */
extern	textline	*tffindline();	/* find a line */
extern	int		tffindcol();	/* find a column */
extern	textline	*tfaddlinebefore();	/* insert a new line */
extern	textgroup	*tffindgroup();	/* find a group that contains a row */
extern	void		tv_findheight();
extern	void		tv_findwidth();

#endif	/* LOOKS_SAME */
