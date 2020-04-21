#include "tf.h"
#include "stdio.h"
/*
 * XXX try keeping an empty line at the end of the frame, and see if we can
 * XXX eliminate some special cases?
 */

/* allocate one thing */
#define	zeroAlloc(type)		(type *) calloc(1, sizeof(type))
#define	tlnew()			zeroAlloc(textline)

#ifdef	DEBUG
void
tfassert(expr, file, line)
	char *expr;
	char *file;
	int line;
{
	fprintf(stderr, "Assertion botch: \"%s\", file \"%s\", line %d\n",
			expr, file, line);
	abort();
}
#endif

/*
 * Make a new text frame
 */
textframe *
tfnew()
{
	textframe *tf;

	tf = zeroAlloc(textframe);
	return (tf);
}

/*
 * Free the storage of a text frame
 */
void
tffree(tf)
	textframe *tf;
{
	register textline *tl, *next;

	tl = tf->tf_first;
	while (tl) {
		next = tl->tl_next;
		if (tl->tl_cdata)
			free(tl->tl_cdata);
		free((char *) tl);
		tl = next;
	}
	free((char *) tf);
}

/*
 * Clone a text frame
 * XXX lets use copy-on-write objects, okay?
 */
textframe *
tfclone(old)
	register textframe *old;
{
	register textframe *new;
	register textline *tl, *tlc;

	new = zeroAlloc(textframe);
	if (new) {
		new->tf_rows = old->tf_rows;
		new->tf_looks = old->tf_looks;
		new->tf_writemask = old->tf_writemask;
		new->tf_point = old->tf_point;
		new->tf_mark = old->tf_mark;
		tl = old->tf_first;
		while (tl) {
			tlc = zeroAlloc(textline);
			tlc->tl_looks = tl->tl_looks;
			tlc->tl_len = tl->tl_len;
			tlc->tl_space = tlc->tl_len;
			tlc->tl_height = tl->tl_height;
			tlc->tl_width = tl->tl_width;
			tlc->tl_dirty = tl->tl_dirty;
			if (tlc->tl_looks & LOOKS_SAME)
				tlc->tl_cdata = (unsigned char *)
					malloc(tlc->tl_space);
			else
				tlc->tl_ldata = (long *)
					malloc(tlc->tl_space * sizeof(long));
			if (new->tf_first)
				new->tf_last->tl_next = tlc;
			else
				new->tf_first = tlc;
			tlc->tl_prev = new->tf_last;
			new->tf_last = tlc;
			tl = tl->tl_next;
		}
	}
	return (new);
}

/*
 * Free a text line.  Return next line in list.
 */
textline *
tl_free(tf, tl)
	register textframe *tf;
	register textline *tl;
{
	register textline *next;

	/* unlink from textframe */
	next = tl->tl_next;
	if (tl->tl_prev)
		tl->tl_prev->tl_next = next;
	if (next)
		next->tl_prev = tl->tl_prev;

	/* fix up textframe first & last pointers */
	if (tl == tf->tf_first)
		tf->tf_first = next;
	if (tl == tf->tf_last)
		tf->tf_last = tl->tl_prev;

	/* free storage */
	if (tl->tl_ldata)
		free((char *) tl->tl_ldata);
	free((char *) tl);
	tf->tf_rows--;
	return (next);
}

/*
 * Free an existing text frame
 */
void
tfree(tf)
	textframe *tf;
{
	register textline *tl;

	tl = tf->tf_first;
	while (tl)
		tl = tl_free(tf, tl);
	free((char *) tf);
}

/*
 * Find the given line
 * XXX create flag needs augmentation
 */
textline *
tf_findline(tf, row, create)
	register textframe *tf;
	register int row;
	int create;
{
	register textline *tl;
	register int newrow;

	if (row < 0)
		return ((textline *) 0);

	if (row >= tf->tf_rows) {
		if (create) {
			while (row >= tf->tf_rows) {
				tl = tlnew();
				if (!tl)
					return ((textline *) 0);
				tl->tl_looks = tf->tf_looks | LOOKS_SAME;
				if (tf->tf_first)
					tf->tf_last->tl_next = tl;
				else
					tf->tf_first = tl;
				tl->tl_prev = tf->tf_last;
				tf->tf_last = tl;
				tf->tf_rows++;
			}
		} else
			return ((textline *) 0);
	}

	/* try easy positions */
	if (row == 0)
		return (tf->tf_first);
	if (row == tf->tf_rows - 1)
		return (tf->tf_last);

	/* couldn't find it the fast way - oh well */
	if (row < tf->tf_rows / 2) {
		/* row is in first half of frame */
		tl = tf->tf_first;
		newrow = 0;
		while (newrow != row) {
			tl = tl->tl_next;
			newrow++;
		}
	} else {
		/* row is in second half of frame */
		tl = tf->tf_last;
		newrow = tf->tf_rows - 1;
		while (newrow != row) {
			tl = tl->tl_prev;
			newrow--;
		}
	}
	return (tl);
}

void
tfdumprowtext(tf, row, f)
	textframe *tf;
	int row;
	FILE *f;
{
	register textline *tl;

	tl = tf_findline(tf, row, 0);
	if (tl) {
		fprintf(f, "Row %5d: len=%3d space=%3d looks=%x\nData: ",
			   row, tl->tl_len, tl->tl_space, tl->tl_looks);
		if (tl->tl_looks & LOOKS_SAME) {
			if (tl->tl_cdata) {
				fwrite(tl->tl_cdata, 1, tl->tl_len, f);
				if (*(tl->tl_cdata + tl->tl_len - 1) != '\n')
					fprintf(f, "(no newline)\n");
			} else {
				fprintf(f, "(no data)\n");
			}
		} else {
			register int i;
			register long *lp;

			lp = tl->tl_ldata;
			for (i = tl->tl_len; --i >= 0; ) {
				fprintf(f, "%x ", *lp++);
			}
			if ((*(tl->tl_ldata + tl->tl_len - 1) & LOOKS_INDEX)
			    != '\n')
				fprintf(f, "(no newline)\n");
			else
				fprintf(f, "\n");
		}
	} else
		fprintf(f, "No row for row %d\n", row);
}

void
tfdumptext(tf, f)
	textframe *tf;
	FILE *f;
{
	register int i;

	for (i = 0; i < tf->tf_rows; i++)
		tfdumprowtext(tf, i, f);
}

void
tfdumpascii(tf, f)
	textframe *tf;
	FILE *f;
{
	register textline *tl;

	tl = tf->tf_first;
	while (tl) {
		if (tl->tl_looks & LOOKS_SAME) {
			fwrite(tl->tl_cdata, 1, tl->tl_len, f);
		} else {
			register int i;
			register long *lp;

			lp = tl->tl_ldata;
			for (i = tl->tl_len; --i >= 0; ) {
				putc(*lp++, f);
			}
		}
		tl = tl->tl_next;
	}
}

/*
 * Make sure the given line exists
 */
void
tfmakeline(tf, row)
	textframe *tf;
	int row;
{
	(void) tf_findline(tf, row, 1);
}

/*
 * Mark the given line dirty so that it gets repainted
 */
void
tftouchline(tf, row)
	textframe *tf;
	int row;
{
	register textline *tl;

	tl = tf_findline(tf, row, 0);
	if (tl) {
		tl->tl_dirty = 1;
		tl->tl_width = 0;		/* XXX needed? */
		tl->tl_height = 0;		/* XXX needed? */
	}
}

/*
 * Get character at row,col from frame
 */
char
tfgetchar(tf, row, col)
    textframe *tf;
    long row, col;
{
    register textline *tl;
    register long looks;
    char c = 0;

    if (col < 0)
	return (c);
    tl = tf_findline(tf, row, 0);
    if (tl) {
	if (tl->tl_looks & LOOKS_SAME) {
	    looks = tl->tl_looks;
	    if (col < tl->tl_len)
		c = *(tl->tl_cdata + col);
	    else
		c = 0;
	} else {
	    if (col < tl->tl_len)
		looks = *(tl->tl_ldata + col);
	    else
		looks = *(tl->tl_ldata + tl->tl_len - 1) & ~LOOKS_INDEX;
	    c = looks & LOOKS_INDEX;
	}
    }
    return (c);
}

/*
 * Grow the textline by inc
 */
void
tl_growline(tf, tl, inc)
	textframe *tf;
	register textline *tl;
	int inc;
{
	ASSERT(inc >= 0);
	if (tl->tl_space >= tl->tl_len + inc)
		return;
	tl->tl_width = 0;			/* invalidate */
	tl->tl_dirty = 1;			/* invalidate */
	if (tl->tl_space == 0) {
		tl->tl_looks = tf->tf_looks | LOOKS_SAME;
		tl->tl_cdata = (unsigned char *) malloc(inc);
		tl->tl_space = inc;
		return;
	}
	if (tl->tl_len == 0)
		tl->tl_looks = tf->tf_looks | LOOKS_SAME;
	tl->tl_space = tl->tl_len + inc;
	if (tl->tl_looks & LOOKS_SAME) {
		tl->tl_cdata = (unsigned char *) realloc(tl->tl_cdata,
							 tl->tl_space);
	} else {
		tl->tl_ldata = (long *) realloc((long *) tl->tl_ldata,
						tl->tl_space * sizeof(long));
	}
}

/*
 * Move data in textline over starting at col for len columns
 */
void
tl_move(tl, startcol, endcol, len)
	register textline *tl;
	int startcol, endcol, len;
{
#ifdef	DEBUG
	if (startcol > endcol) {
		/* contraction move */
		ASSERT(tl->tl_space >= startcol + len);
	} else {
		/* expansion move */
		ASSERT(tl->tl_space >= endcol + len);
	}
#endif
	if (tl->tl_looks & LOOKS_SAME) {
		BCOPY(tl->tl_cdata + startcol, tl->tl_cdata + endcol, len);
	} else {
		BCOPY(tl->tl_ldata + startcol, tl->tl_ldata + endcol,
				   len * sizeof(long));
	}

	/* these are probably redundant, but better safe than sorry */
	tl->tl_dirty = 1;
	tl->tl_width = 0;
}

/*
 * Convert a text line from one format to the other
 */
void
tl_cvtline(tl)
	register textline *tl;
{
	register long *lp;
	register unsigned char *cp;
	register int i;

	ASSERT(tl->tl_space > 0);
	if (tl->tl_looks & LOOKS_SAME) {
		long *savelp;

		/*
		 * Convert to long format
		 */
		cp = tl->tl_cdata;
		lp = (long *) malloc(tl->tl_space * sizeof(long));
		savelp = lp;
		for (i = tl->tl_len; --i >= 0; )
			*lp++ = *cp++ | tl->tl_looks;
		free((char *) tl->tl_cdata);
		tl->tl_ldata = savelp;
		tl->tl_looks &= ~LOOKS_SAME;
	} else {
		unsigned char *savecp;

		/*
		 * Convert to short format
		 */
		lp = tl->tl_ldata;
		cp = (unsigned char *) malloc(tl->tl_space);
		savecp = cp;
		for (i = tl->tl_len; --i >= 0; )
			*cp++ = *lp++;
		free((char *) tl->tl_ldata);
		tl->tl_cdata = savecp;
		tl->tl_looks |= LOOKS_SAME;
	}
}

/*
 * Copy character data into the line to the given column
 */
void
tl_copychars(tl, startcol, buf, len, looks)
	register textline *tl;
	register int startcol, len;
	register unsigned char *buf;
	long looks;
{
	ASSERT(tl->tl_space >= startcol + len);

	if ((tl->tl_looks & LOOKS_SAME) &&
	    ((tl->tl_looks & LOOKS_MASK) != (looks & LOOKS_MASK)))
		tl_cvtline(tl);

	tl->tl_dirty = 1;
	tl->tl_width = 0;
	if (tl->tl_looks & LOOKS_SAME) {
		BCOPY(buf, tl->tl_cdata + startcol, len);
	} else {
		register int i;
		register long *lp;

		lp = tl->tl_ldata + startcol;
		while (--len >= 0) {
			*lp++ = *buf++ | looks;
		}
		tl->tl_height = 0;
	}
}

/*
 * Copy long data into the line to the given column
 */
/*ARGSUSED*/
void
tl_copylongs(tl, startcol, buf, len, looks)
	register textline *tl;
	int startcol, len;
	long *buf;
{
	ASSERT(tl->tl_space >= startcol + len);
	if (tl->tl_looks & LOOKS_SAME)
		tl_cvtline(tl);
	BCOPY(buf, tl->tl_ldata + startcol, len * sizeof(long));
	tl->tl_dirty = 1;
	tl->tl_width = 0;
	tl->tl_height = 0;
}

/*
 * Put the given buffer into the text frame.  Use "func" to actually
 * copy the data
 */
int
tl_putit(tf, buf, len, func)
	register textframe *tf;
	char *buf;
	int len;
	int (*func)();
{
	register textline *tl;
	register int col;

	tl = tf_findline(tf, tf->tf_point.tc_row, 1);
	if (!tl)
		return (-1);

	/* make line bigger */
	tl_growline(tf, tl, len);

	/* bring point's column in bounds */
	col = tf->tf_point.tc_col;
	if (col < 0)
		col = 0;
	if (col > tl->tl_len) {
		col = tl->tl_len;
		tf->tf_point.tc_col = col;
	} else
	if (col != tl->tl_len) {
		tl_move(tl, col, col + len, tl->tl_len - col);
	}
	tf->tf_mark.tc_row = tf->tf_point.tc_row;
	tf->tf_mark.tc_col = col;

	/* copy data into line */
	tl->tl_len += len;
	(*func)(tl, col, buf, len, tf->tf_looks);

	/* adjust location of point */
	tf->tf_point.tc_col += len;
	return (0);
}

tfputascii(tf, buf, len)
	textframe *tf;
	char *buf;
	int len;
{
	return (tl_putit(tf, buf, len, tl_copychars));
}

tfputtext(tf, buf, len)
	textframe *tf;
	long *buf;
	int len;
{
	return (tl_putit(tf, buf, len, tl_copylongs));
}

#ifdef	NOTDEF
/*
 * Put the given buffer into the text frame.  Same as tfputascii, except
 * the data overwrites any existing data.
 * XXX ARRAY-OP
 * XXX needs to pad, if column is past end of line before starting
 */
int
tl_wrtit(tf, buf, len, func)
	register textframe *tf;
	char *buf;
	int len;
	int (*func)();
{
	register textline *tl;
	register int col;
	register int newlen;

	tl = tf_findline(tf, tf->tf_point.tc_row, 1);
	if (!tl)
		return (-1);

	/* bring point's column in bounds */
	col = tf->tf_point.tc_col;
	if (col < 0)
		col = 0;
	if (col > tl->tl_len) {
		/* XXX wrong, for arrays */
		col = tl->tl_len;
		tf->tf_point.tc_col = col;
	}

	/* make line bigger, if needed */
	if (col + len > tl->tl_len) {
		newlen = col + len;
		tl_growline(tf, tl, newlen - tl->tl_len);
		tl->tl_len = newlen;
		tl->tl_width = 0;
	}

	/* copy data into line */
	(*func)(tl, col, buf, len, tf->tf_looks);

	/* adjust location of point */
	tf->tf_point.tc_col += len;
	return (0);
}

int
tfwrtascii(tf, buf, len)
	textframe *tf;
	char *buf;
	int len;
{
	return (tl_wrtit(tf, buf, len, tl_copychars));
}

int
tfwrttext(tf, buf, len)
	textframe *tf;
	long *buf;
	int len;
{
	return (tl_wrtit(tf, buf, len, tl_copylongs));
}
#endif

/*
 * Sort point and mark
 */
int
tf_sortpm(p, m)
	register textcoord *p, *m;
{
	register long temp;

	if (p->tc_row == m->tc_row) {
		if (p->tc_col > m->tc_col) {
			temp = m->tc_col;
			m->tc_col = p->tc_col;
			p->tc_col = temp;
		}
	} else
	if (p->tc_row > m->tc_row) {
		temp = m->tc_row;
		m->tc_row = p->tc_row;
		p->tc_row = temp;
		temp = m->tc_col;
		m->tc_col = p->tc_col;
		p->tc_col = temp;
	}

	/* return non-zero if point/mark are invalid */
	if ((p->tc_row < 0) || (m->tc_row < 0) ||
	    (p->tc_col < 0) || (m->tc_col < 0))
		return (1);
	return (0);
}

/*
 * Delete between point and mark
 */
void
tfdelete(tf)
	register textframe *tf;
{
	register textline *tl;
	register textline *point, *mark;
	register int delta;
	textcoord p, m;

	/* validate point and mark */
	p = tf->tf_point;
	m = tf->tf_mark;
	if (tf_sortpm(&p, &m))
		return;

	/* get point and mark positions in frame */
	point = tf_findline(tf, p.tc_row, 0);
	if (!point)
		return;
	if (p.tc_col >= point->tl_len)
		p.tc_col = point->tl_len;
	mark = tf_findline(tf, m.tc_row, 0);

	/*
	 * Delete lines between point and mark, exclusively.  Mark is allowed
	 * to be past the end of the frame, which means that the the
	 * last line in the frame can get deleted.
	 */
	if (point == mark) {
		/*
		 * If point and mark are at the same location, or if the
		 * point is past the end of the line, then this is a nop.
		 */
		if ((p.tc_col == m.tc_col) ||
		    (p.tc_col >= point->tl_len))
			return;				/* nop */
		/*
		 * If mark is past the end of the line, then just truncate
		 * the line at the point.
		 */
		point->tl_dirty = 1;
		point->tl_width = 0;
		if (m.tc_col >= point->tl_len) {
			point->tl_len = p.tc_col;
			return;
		}
		/*
		 * Slide chars to the right of the mark over to where the
		 * point is.
		 */
		tl_move(point, m.tc_col, p.tc_col, point->tl_len - m.tc_col);
		point->tl_len -= m.tc_col - p.tc_col;
	} else {
		/*
		 * Delete the whole lines following point, up to but not
		 * including the mark.
		 */
		tl = point->tl_next;
		while (tl != mark)
			tl = tl_free(tf, tl);

		/* truncate point line */
		point->tl_len = p.tc_col;
		point->tl_dirty = 1;
		point->tl_width = 0;

		/* if no mark line, then just truncate the point line */
		if (!mark)
			return;
		if (m.tc_col >= mark->tl_len)
			m.tc_col = mark->tl_len;
		ASSERT(point->tl_next == mark);

		/*
		 * Grow point line, and then copy data from mark line to
		 * it.  Delete mark line and update point line length.
		 */
		delta = mark->tl_len - m.tc_col;
		tl_growline(tf, point, delta);
		if (mark->tl_looks & LOOKS_SAME)
			tl_copychars(point, p.tc_col,
					    mark->tl_cdata + m.tc_col,
					    delta, mark->tl_looks);
		else
			tl_copylongs(point, p.tc_col,
					    mark->tl_ldata + m.tc_col,
					    delta, 0);
		point->tl_len += delta;
		(void) tl_free(tf, mark);
	}
}

/*
 * Change looks in a line between start and end columns
 */
/*ARGSUSED*/
int
tl_changelooks(tf, tl, startcol, endcol, buf, len)
	register textframe *tf;
	register textline *tl;
	register int startcol, endcol;
	unsigned char **buf;
	int *len;
{
	int count;

	count = startcol - endcol;

	/* check for quick change */
	if ((startcol == 0) &&
	    (endcol == tl->tl_len) &&
	    (tl->tl_looks & LOOKS_SAME)) {
		tl->tl_looks = (tl->tl_looks & tf->tf_writemask) | tf->tf_looks;
	} else {
		register long *lp;

		/* oh well, convert to long form */
		if (tl->tl_looks & LOOKS_SAME)
			tl_cvtline(tl);

		/* update looks in span */
		lp = tl->tl_ldata + startcol;
		while (startcol < endcol) {
			*lp++ = (*lp & tf->tf_writemask) | tf->tf_looks;
			startcol++;
		}
		/* XXX should try to undo tl_cvtline effect, if possible */
	}
	tl->tl_dirty = 1;
	/* XXX could optimize dirty bits a touch */
	tl->tl_width = 0;
	tl->tl_height = 0;
	return (count);
}

/*
 * Get ascii data out of buffer.  Return count of characters copied.
 */
/*ARGSUSED*/
int
tl_getascii(tf, tl, startcol, endcol, bufp, lenp)
	textframe *tf;
	textline *tl;
	int startcol, endcol;
	register unsigned char **bufp;
	register int *lenp;
{
	register int amount;
	int count;

	/* figure limit to copy */
	amount = endcol - startcol;
	if (amount > *lenp)
		amount = *lenp;
	count = amount;

	if (tl->tl_looks & LOOKS_SAME) {
		BCOPY(tl->tl_cdata + startcol, *bufp, amount);
	} else {
		register long *lp;
		register unsigned char *cp;

		lp = tl->tl_ldata + startcol;
		cp = *bufp;
		while (--amount >= 0) {
			*cp++ = *lp++;
		}
	}
	*bufp += count;
	*lenp -= count;
	return (count);
}

/*
 * Get text data out of buffer.  Return count of longs copied.
 */
/*ARGSUSED*/
int
tl_gettext(tf, tl, startcol, endcol, bufp, lenp)
	textframe *tf;
	textline *tl;
	int startcol, endcol;
	register long **bufp;
	register int *lenp;
{
	register int amount;
	int count;

	/* figure limit to copy */
	amount = endcol - startcol;
	if (amount > *lenp)
		amount = *lenp;
	count = amount;

	if (tl->tl_looks & LOOKS_SAME) {
		register long *lp;
		register unsigned char *cp;

		cp = tl->tl_cdata + startcol;
		lp = *bufp;
		while (--amount >= 0) {
			*lp++ = *cp++ | tl->tl_looks;
		}
	} else {
		BCOPY(tl->tl_ldata + startcol, *bufp, amount * sizeof(long));
	}
	*bufp += count;
	*lenp -= count;
	return (count);
}

/*
 * Dummy function to return length of endcol-startcol.  Used for tfselcount().
 */
/* ARGSUSED */
int
tl_selcount(tf, tl, startcol, endcol, buf, len)
	textframe *tf;
	textline *tl;
	int startcol, endcol;
	unsigned char **buf;
	int *len;
{
	return (endcol - startcol);
}

/*
 * Change looks between point and mark
 */
int
tf_pmop(tf, buf, len, func)
	textframe *tf;
	unsigned char *buf;
	int len;
	int (*func)();
{
	register textline *point, *mark;
	register textline *tl;
	register int count, inc;
	textcoord p, m;

	/* validate point and mark */
	count = 0;
	p = tf->tf_point;
	m = tf->tf_mark;
	if (tf_sortpm(&p, &m))
		return (count);

	/* find point & mark */
	point = tf_findline(tf, p.tc_row, 0);
	if (!point)
		return (count);
	mark = tf_findline(tf, m.tc_row, 0);

	if (point == mark) {
		if ((p.tc_col < point->tl_len) &&
		    (p.tc_col != m.tc_col)) {
			if (m.tc_col > point->tl_len)
				m.tc_col = point->tl_len;
			count += (*func)(tf, point, p.tc_col, m.tc_col,
					     &buf, &len);
		}
	} else {
		/* apply function to data on point line */
		if (p.tc_col < point->tl_len) {
			inc = (*func)(tf, point, p.tc_col, point->tl_len,
					  &buf, &len);
			if (inc == 0)
				return (count);
			count += inc;
		}

		/*
		 * Apply function to each line between point and mark,
		 * exclusively
		 */
		tl = point->tl_next;
		while (tl != mark) {
			inc = (*func)(tf, tl, 0, tl->tl_len, &buf, &len);
			if (inc == 0)
				return (count);
			count += inc;
			tl = tl->tl_next;
		}

		/* apply function to data on mark line, if any */
		if (mark && (m.tc_col < mark->tl_len))
			count += (*func)(tf, mark, 0, m.tc_col, &buf, &len);
	}
	return (count);
}

int
tfchangelooks(tf)
	textframe *tf;
{
	return (tf_pmop(tf, (unsigned char *) 0, 0, tl_changelooks));
}

int
tfselcount(tf)
	textframe *tf;
{
	return (tf_pmop(tf, (unsigned char *) 0, 0, tl_selcount));
}

int
tfgetascii(tf, buf, len)
	textframe *tf;
	char *buf;
	int len;
{
	return (tf_pmop(tf, (unsigned char *) buf, len, tl_getascii));
}

int
tfgettext(tf, buf, len)
	textframe *tf;
	long *buf;
	int len;
{
	return (tf_pmop(tf, (unsigned char *) buf, len, tl_gettext));
}

/*
 * Split the line at point.tc_col
 */
void
tfsplit(tf)
	register textframe *tf;
{
	register textline *tl, *newtl;
	register int col;
	register int delta;

	tl = tf_findline(tf, tf->tf_point.tc_row, 0);
	if (!tl)
		return;
	col = tf->tf_point.tc_col;
	if (col < 0)
		return;
	if (col > tl->tl_len)
		col = tl->tl_len;

	/* allocate a new line and allocate space for the split off data */
	delta = tl->tl_len - col;
	newtl = tlnew();
	tl_growline(tf, newtl, delta);
	newtl->tl_len = delta;
	tl->tl_len = col;
	tl->tl_dirty = 1;
	tl->tl_width = 0;
	tl->tl_height = 0;

	/* copy data from old line to new line */
	if (tl->tl_looks & LOOKS_SAME) {
		newtl->tl_looks = tl->tl_looks;
		BCOPY(tl->tl_cdata+col, newtl->tl_cdata, delta);
	} else {
		tl_cvtline(newtl);
		BCOPY(tl->tl_ldata+col, newtl->tl_ldata, delta * sizeof(long));
	}

	/* insert new line after old line */
	if (newtl->tl_next = tl->tl_next)
		tl->tl_next->tl_prev = newtl;
	newtl->tl_prev = tl;
	tl->tl_next = newtl;
	if (tf->tf_last == tl)
		tf->tf_last = newtl;
	tf->tf_rows++;

	/* XXX invalidate textline pointers below this line */
	/* XXX set dirty-below flag */

	/* move point to beginning of new line */
	tf->tf_point.tc_row++;
	tf->tf_point.tc_col = 0;
}

void
tfbackcolor(tf, bg)
	textframe *tf;
	long bg;
{
	tf->tf_looks &= ~LOOKS_BG;
	tf->tf_looks |= bg << LOOKS_BGSHIFT;
}

void
tftextcolor(tf, fg)
	textframe *tf;
	long fg;
{
	tf->tf_looks &= ~LOOKS_FG;
	tf->tf_looks |= fg << LOOKS_FGSHIFT;
}

void
tffont(tf, fonthandle)
	textframe *tf;
	long fonthandle;
{
	tf->tf_looks &= ~LOOKS_FONT;
	tf->tf_looks |= fonthandle << LOOKS_FONTSHIFT;
}

void
tfgetlooks(tf, looks)
	textframe *tf;
	long *looks;
{
	*looks = tf->tf_looks & LOOKS_MASK;
}

void
tfgetmark(tf, row, col)
	textframe *tf;
	long *row;
	long *col;
{
	*row = tf->tf_mark.tc_row;
	*col = tf->tf_mark.tc_col;
}

void
tfgetpoint(tf, row, col)
	textframe *tf;
	long *row;
	long *col;
{
	*row = tf->tf_point.tc_row;
	*col = tf->tf_point.tc_col;
}

long
tfgetwritemask(tf)
	textframe *tf;
{
	return (~tf->tf_writemask);
}

long
tfnumcols(tf, row)
	register textframe *tf;
	register long row;
{
	register textline *tl;

	if (tl = tf_findline(tf, row, 0))
		return (tl->tl_len);
	else
		return (0);
}

long
tfnumrows(tf)
	textframe *tf;
{
	return (tf->tf_rows);
}

void
tfsetlooks(tf, looks)
	textframe *tf;
	long looks;
{
	tf->tf_looks = looks & LOOKS_MASK;
}

void
tfsetmark(tf, row, col)
	textframe *tf;
	short row, col;
{
	tf->tf_mark.tc_row = row;
	tf->tf_mark.tc_col = col;
}

void
tfsetpoint(tf, row, col)
	textframe *tf;
	short row, col;
{
	tf->tf_point.tc_row = row;
	tf->tf_point.tc_col = col;
}

void
tfsetwritemask(tf, mask)
	textframe *tf;
	long mask;
{
	tf->tf_writemask = ~(mask & LOOKS_MASK);
}
