/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/srcwin.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:06 $
 */

#include <gl.h>
#include <device.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "window.h"
#include "tf.h"
#include "manage.h"

textframe *tf;
textview *tv;
int xsize, ysize;
int	srcwin;
extern	short	charwidth, charheight, chardescender;
int	selecting;
static	int	xorg, yorg;
SIZE_DESC	src_rect = { 6, 143, 80, 22, 1, 1, 1 };

#define CHANGED 0
#define HILIGHT   0
#define DEHILIGHT 1
#define MARGINSZ 20
#define REDBG (RED << LOOKS_BGSHIFT)
#define GREENBG (GREEN << LOOKS_BGSHIFT)
#define CYANBG (CYAN << LOOKS_BGSHIFT)
#define DEF_BG (dpagecolor << LOOKS_BGSHIFT)

typedef	struct brkpt {
	long	lineno;
	int	event_id;
	struct	brkpt	*nextbp;
} BRKPT;


	    /* methods for drawing and undrawing text input point */
#define OFF 0
#define ON  1

typedef struct {
    int x, y;
    int inbounds;
} locate;

typedef struct {
	int col, row;
} colrow;

typedef struct {
	colrow	mark;
	colrow	point;
} SELECTION;


typedef	struct srcview {
	textframe	*tf;
	textview	*tv;
	long	looks;
	long	topline;
	long	pagetop;
	SELECTION	curline;
	long	nlines;
	long	viewlines;
	char	*filename;
	BRKPT	*brkpts;
	time_t	date;
	struct	srcview	*nextsv;
} SRCVIEW;


SRCVIEW	*gsvp;

#define HASHSIZE 100
SRCVIEW	*srcviews[HASHSIZE];

/*
 * Hash the filename into an index into srcviews.
 */
int
hash(filename) 
char	*filename;
{

	int	hashval = 0;
	int	i, j;
	
	i = strlen(filename);
	for (j = 0; j < i; j++) {
		hashval = hashval + (int) filename[j];
	}
	return(hashval % HASHSIZE);
}


/*
 * Find the srcview associated with filename
 */
SRCVIEW	*
findsrcview(filename)
char	*filename;
{
	int	hashval;
	SRCVIEW	*svp;

	hashval = hash(filename);
	svp = srcviews[hashval];
	while (svp != (SRCVIEW *) 0) {
		if (strcmp(svp->filename, filename) == 0) {
			return(svp);
		}
		svp = svp->nextsv;
	}
	return((SRCVIEW *) 0);
}


/*
 * Add a source file to the srcview table
 * makeing sure to look for hash collision.
 */
addsrcfile(filename)
char	*filename;
{
	struct	stat	statbuf;
	int	hashval;
	SRCVIEW	*sp;
	SRCVIEW	*svp;
	SRCVIEW	*lastsvp;
	char	*malloc();

	
	if ((svp = findsrcview(filename)) != (SRCVIEW *) 0) {
		return;
	}
	hashval = hash(filename);
	lastsvp = (SRCVIEW *) 0;
	for (svp = srcviews[hashval]; svp != (SRCVIEW *) 0;
		svp = svp->nextsv) {
		lastsvp = svp;
	}
	if (lastsvp == (SRCVIEW *) 0) {
		srcviews[hashval] = (SRCVIEW *) malloc(sizeof(SRCVIEW));
		sp = srcviews[hashval];
	} else {
		lastsvp->nextsv = (SRCVIEW *) malloc(sizeof(SRCVIEW));
		sp = lastsvp->nextsv;
	}
	sp->filename = malloc(strlen(filename) + 1);
	strcpy(sp->filename, filename);
	stat(sp->filename, &statbuf);
	sp->date = statbuf.st_mtime;
	sp->tv = (textview *) 0;
	sp->tf = (textframe *) 0;
	sp->nextsv = 0;
	sp->brkpts = (BRKPT *) 0;
}
	
	



init_srcwin(filename)
char	*filename;
{
	int	*p;
	int	i;
	int	cur_win;
	SRCVIEW	*svp;
	struct	stat	statbuf;
	char	title_buf[512];

	if ((svp = findsrcview(filename)) == (SRCVIEW *) 0) {
		addsrcfile(filename);
		svp = findsrcview(filename);
	}
	
	gsvp = svp;
	svp->tf = tfnew();
	tfsetlooks(svp->tf, (0 << LOOKS_FONTSHIFT) |
		   (dtextcolor << LOOKS_FGSHIFT) |
		   (dpagecolor << LOOKS_BGSHIFT));

	svp->looks = mytfgetlooks(svp->tf);

/*
	if (flag_font) {
		tffont(svp->tf, 1);
	}
*/
	svp->nlines = readascii(svp->tf, filename);
	stat(svp->filename, &statbuf);
	svp->date = statbuf.st_mtime;


	cur_win = winget();
	if (srcwin == 0) {
		reset_constraints(WT_SRC);
		srcwin = winopen(filename);
		winset(srcwin);
		lsrepeat(1);
		writemask(0xffff);
		winconstraints();
		stepunit(charheight, charwidth);
		winconstraints();
		getorigin(&xorg, &yorg);
	} else {
		winset(srcwin);
	}
	sprintf(title_buf, "edge: Source Window: %s", filename);
	wintitle(title_buf);
	svp->tv = tvnew(svp->tf);
		lsrepeat(1);
	tvleftpix(svp->tv, MARGINSZ);
	getsize(&xsize, &ysize);
	/*
	if (flag_font) {
		tvmapfont(svp->tv, 1, flag_font);
	}
	*/
	svp->topline = 0;
	svp->viewlines = (ysize / charheight) - 1;
	tvviewsize(svp->tv, xsize, ysize);
	tvsetbg(svp->tv, dpagecolor);
	tvsetstops(svp->tv, 8*8, 0);
	tvtoprow(svp->tv,0);
	tvhighlight(svp->tv, CYAN, 0);
	color_bps(svp);
	tvdraw(svp->tv,1);
}


display(lineno, filename) 
int	lineno;
char	*filename;
{
	long	looks;
	int	cur_win;
	SRCVIEW	*svp;
	struct	stat	statbuf;

	if (srcwin == 0) {
		init_srcwin(filename);
	}
	cur_win = winget();
	winset(srcwin);
	svp = findsrcview(filename);
	stat(filename, &statbuf);

	if ((svp != gsvp) || (gsvp->date < statbuf.st_mtime)) {
		tffree(gsvp->tf);
		tvfree(gsvp->tv);
		gsvp->tf = (textframe *) 0;
		init_srcwin(filename);
		svp = gsvp;
	}
	if (lineno != -1) {
		if ((lineno < svp->topline) || 
			(lineno > (svp->topline + svp->viewlines))) {
			if ((lineno - (svp->viewlines / 2)) <= 0) {
				svp->topline = 0;
				svp->pagetop = 0;
				tvtoprow(svp->tv, 0);
			} else {
				svp->topline = lineno - (svp->viewlines / 2);
				svp->pagetop = svp->topline;
				tvtoprow(svp->tv, svp->topline);
			}
		}
	}
	if (isbrkpt(svp, svp->curline.mark.row)) {
/*
		if (svp->curline.mark.row == tmark.row) {
		}
*/
		setcolor(svp->tf, svp->tv, REDBG, &(svp->curline.mark), 
			&(svp->curline.point));
	} else {
		unsetcolor(svp->tf, svp->tv, &(svp->curline.mark), 
			&(svp->curline.point));
	}
	if (lineno != -1) {
		svp->curline.mark.row = lineno;
		svp->curline.mark.col= 0;
		svp->curline.point.row = lineno;
		svp->curline.point.col= tfnumcols(svp->tf, lineno);
/*
		if (svp->curline.mark.row == tmark.row) {
		}
*/
		setcolor(svp->tf, svp->tv, GREENBG, &(svp->curline.mark), 
			&(svp->curline.point));
	}
	/*
	winset(cur_win);
	*/
		
}

mk_topline(filename, lineno)
char	*filename;
int	lineno;
{
	int	cur_win;
	SRCVIEW	*svp;

	winset(srcwin);
	svp = findsrcview(filename);

	if (svp != gsvp) {
		tffree(gsvp->tf);
		tvfree(gsvp->tv);
		gsvp->tf = (textframe *) 0;
		init_srcwin(filename);
		svp = gsvp;
	}
	svp->topline = lineno;
	tvtoprow(svp->tv, svp->topline);
	drawtext(srcwin, svp->tv, CHANGED);
}

mk_midline(filename, lineno)
char	*filename;
int	lineno;
{
	int	cur_win;
	SRCVIEW	*svp;
	colrow	mark, point;

	winset(srcwin);
	svp = findsrcview(filename);

	if (svp != gsvp) {
		tffree(gsvp->tf);
		tvfree(gsvp->tv);
		gsvp->tf = (textframe *) 0;
		init_srcwin(filename);
		svp = gsvp;
	}
	if ((lineno < svp->topline) || 
		(lineno > (svp->topline + svp->viewlines))) {
		if ((lineno - (svp->viewlines / 2)) <= 0) {
			svp->topline = 0;
			tvtoprow(svp->tv, 0);
		} else {
			svp->topline = lineno - (svp->viewlines / 2);
			tvtoprow(svp->tv, svp->topline);
		}
	}
	mark.row = lineno;
	mark.col = 0;
	point.row = lineno;
	point.col = tfnumcols(svp->tf, lineno);
	
	selhilight(svp->tf, svp->tv, HILIGHT, &mark, &point);
	drawtext(srcwin, svp->tv, CHANGED);
	sginap(50);
	selhilight(svp->tf, svp->tv, DEHILIGHT, &mark, &point);
	drawtext(srcwin, svp->tv, CHANGED);
}

selhilight(tfp, tv,  action, end1, end2)
	textframe	*tfp;
	textview	*tv;
	int action;
	colrow *end1, *end2;
{
	long	looks;
	long	oldlooks;

	oldlooks = mytfgetlooks(tfp);
	switch (action) {
	case HILIGHT:
	looks = (gsvp->looks & ~LOOKS_BG) | (CYANBG & (LOOKS_BG));
/*
		looks = LOOKS_BG;
*/
		break;
	case DEHILIGHT:
		looks = (gsvp->looks & ~LOOKS_BG) | (dpagecolor<<LOOKS_BGSHIFT & (LOOKS_BG));
		break;

	default:
		return;
	}

	tfsetmark(tfp, end1->row, end1->col);
	tfsetpoint(tfp, end2->row, end2->col);

	tfsetlooks(tfp, looks);
	tfsetwritemask(tfp, LOOKS_BG);
	tfchangelooks(tfp);
	drawtext(srcwin, tv, CHANGED);
	tfsetlooks(tfp, oldlooks);
}

 
setcolor(tf, tv, color, begin, end)
textframe	*tf;
textview	*tv;
int	color;
colrow	*begin, *end;
{
	long	looks;
	long	oldlooks;

	oldlooks = mytfgetlooks(tf);
	looks = (gsvp->looks & ~LOOKS_BG) | (color & (LOOKS_BG));
	tfsetlooks(tf, looks);
	tfsetmark(tf, begin->row, begin->col);
	tfsetpoint(tf, end->row, end->col);
	tfsetwritemask(tf, LOOKS_BG);
	tfchangelooks(tf);
	drawtext(srcwin, tv, CHANGED);
	tfsetlooks(tf, oldlooks);
}

unsetcolor(tf, tv, begin, end)
textframe	*tf;
colrow	*begin, *end;
{
	long	looks;
	long	oldlooks;

	oldlooks = mytfgetlooks(tf);
	looks = (gsvp->looks & ~LOOKS_BG) | (DEF_BG & (LOOKS_BG));
	tfsetlooks(tf, looks);
	tfsetlooks(tf, (0 << LOOKS_FONTSHIFT) |
		   (dtextcolor << LOOKS_FGSHIFT) |
		   (dpagecolor << LOOKS_BGSHIFT));
	tfsetmark(tf, begin->row, begin->col);
	tfsetpoint(tf, end->row, end->col);
	tfsetwritemask(tf, LOOKS_BG);
	tfchangelooks(tf);
	drawtext(srcwin, tv, CHANGED);
	tfsetlooks(tf, oldlooks);
}

drawtext(window, tv, how)
int	window;
textview	*tv;
long how;
{
	int	cur_win;

	winset(window);
	tvdraw(tv, how);

}

srcredisp() {
/*
	int	xsize, ysize;
*/
	int	cur_win;

	cur_win = winget();
	winset(srcwin);
	getsize(&xsize, &ysize);
	getorigin(&xorg, &yorg);
	gsvp->looks = mytfgetlooks(gsvp->tf);
	color((gsvp->looks & LOOKS_BG) >> LOOKS_BGSHIFT);
	rectfi(0, 0, MARGINSZ, ysize);
	gsvp->viewlines = (ysize / charheight) - 1;
	tvviewsize(gsvp->tv, xsize, ysize);
	drawtext(srcwin, gsvp->tv, 1);
}


    /*
     * read lines from a file of ascii characters into a textframe,
     * without performing wrapping.
     */
readascii(tf, name)
    textframe *tf;
    char *name;
{
    FILE *fd;
    char oneline[1032];
    long count;
    textcoord begin;
	char	numbuf[6];
	long	linenum = 1;
	int	i;

	for (i = 0; i < 8; i++) {
		oneline[i] = ' ';
	}
    if (!(fd = fopen(name, "r"))) {
	fprintf(stderr,"can't open file %s\n", name);
	return -1;
    }
    tfgetpoint(tf, &begin.tc_row, &begin.tc_col);
    while (fgets(&(oneline[i]), 1024, fd)) {
	if (count = strlen(oneline)) {
		sprintf(numbuf, "%d", linenum);
		strcpy(oneline, numbuf);
		oneline[strlen(numbuf)] = ' ';
		linenum++;
	    tfputascii(tf, oneline, count);
	    if (oneline[count-1] == '\n')
		tfsplit(tf);
	}
    }
    tfsetmark(tf, begin.tc_row, begin.tc_col);
    fclose(fd);
    return 0;
}



scrollwin(win_id, val)
int	win_id;
int	val;
{


	winset(win_id);
	if (win_id == srcwin) {
		scrollsrc(val);
	} else if (win_id == var_win) {
		scrollvar(val);
	}
}

scrollsrc(val)
int	val;
{
	float	pct;

	if ((val < yorg) || (val > (yorg + ysize))) {
		unqdevice(MOUSEX);
		unqdevice(MOUSEY);
		scrolling = 0;
		scroll_win = -1;
	}
		
		
	pct = ((float) (ysize) - (float) (val - yorg)) / (float) (ysize);
	gsvp->topline = tfnumrows(gsvp->tf) * pct;
	tvtoprow(gsvp->tv, gsvp->topline);
	tvdraw(gsvp->tv, 0);
}

long
mytfgetlooks(tf)
textframe	*tf;
{
	long	looks;

	tfgetlooks(tf, &looks);
	return(looks);
#ifdef notdef
	return (tf->tf_looks);
#endif
}	


color_bps(svp)
SRCVIEW	*svp;
{
	BRKPT	*bpp;
	colrow	mark, point;

	mark.col = 0;
	for (bpp = svp->brkpts; bpp != (BRKPT *) 0; bpp = bpp->nextbp) {
		mark.row = bpp->lineno - 1;
		point.row = bpp->lineno - 1;
		point.col = tfnumcols(svp->tf, bpp->lineno - 1);
		setcolor(svp->tf, svp->tv, REDBG, &mark, 
			&point);
	}
}

int
isbrkpt(svp, row)
SRCVIEW	*svp;
int	row;
{
	BRKPT	*bpp;

	for (bpp = svp->brkpts; bpp != (BRKPT *) 0; bpp = bpp->nextbp) {
		if (bpp->lineno == row + 1) {
			return(1);
		}
	}
	return(0);
}

del_bp(event_id)
int	event_id;
{
	SRCVIEW	*svp;
	int	i;

	for (i = 0; i < HASHSIZE; i++) {
		for (svp = srcviews[i]; svp != (SRCVIEW *) 0;
			svp = svp->nextsv) {
			do_delete(svp, event_id);
		}
	}
}

do_delete(svp, event_id)
SRCVIEW	*svp;
int	event_id;
{

	BRKPT	*bpp, *last_bp;

	struct	stat	statbuf;
	last_bp = (BRKPT *) 0;
	for (bpp = svp->brkpts; 
			bpp != (BRKPT *) 0; bpp = bpp->nextbp) {
		if (bpp->event_id == event_id) {
			if (last_bp != (BRKPT *) 0) {
				last_bp->nextbp = bpp->nextbp;
			} else if (bpp->nextbp != (BRKPT *) 0)  {
				svp->brkpts = bpp->nextbp;
			} else {
				svp->brkpts = (BRKPT *) 0;
			}
			if (svp == gsvp) {
				colrow	mark, point;
				stat(svp->filename, &statbuf);
				if (gsvp->date < statbuf.st_mtime) {
					tffree(gsvp->tf);
					tvfree(gsvp->tv);
					gsvp->tf = (textframe *) 0;
					init_srcwin(gsvp->filename);
					svp = gsvp;
				}
				mark.row = bpp->lineno - 1;
				point.row = bpp->lineno - 1;
				mark.col = 0;
				point.col = tfnumcols(svp->tf, bpp->lineno - 1);
				unsetcolor(svp->tf, svp->tv, 
					&mark, &point);
			}
		}
		last_bp = bpp;
	}
}

add_bp(filename, lineno, event_id)
char	*filename;
int	lineno;
int	event_id;
{
	SRCVIEW	*svp;
	BRKPT	*bpp;
	colrow	begin, end;
	long	row;
	struct	stat	statbuf;

	row = lineno - 1;
	if ((svp = findsrcview(filename)) == (SRCVIEW *) 0) {
		addsrcfile(filename);
		svp = findsrcview(filename);
	}
	if (svp->brkpts == (BRKPT *) 0) {
		svp->brkpts = (BRKPT *) malloc(sizeof(BRKPT));
		bpp = svp->brkpts;
	} else {
		bpp = svp->brkpts;
		while (bpp) {
			if (bpp->nextbp == (BRKPT *) 0) {
				break;
			}
			bpp = bpp->nextbp;
		}
		bpp->nextbp = (BRKPT *) malloc(sizeof(BRKPT));
		bpp = bpp->nextbp;
	}
	bpp->nextbp = (BRKPT *) 0;
	bpp->lineno = lineno;
	bpp->event_id = event_id;
	if (svp == gsvp) {
		stat(svp->filename, &statbuf);
		if (gsvp->date < statbuf.st_mtime) {
			tffree(gsvp->tf);
			tvfree(gsvp->tv);
			gsvp->tf = (textframe *) 0;
			init_srcwin(gsvp->filename);
			svp = gsvp;
		}
		begin.col = 0;
		end.col = tfnumcols(svp->tf, row);
		begin.row = row;
		end.row = row;
		setcolor(svp->tf, svp->tv, REDBG, &begin, &end); 
	}
}


getbg(tf)
textview	*tf;
{
	long	looks;

	looks = mytfgetlooks(tf);
	return((looks & LOOKS_BG) >> LOOKS_BGSHIFT);
}

do_select(val)
int	val;
{
	int	inbounds;
	locate	loc;
	int	curwin;

	winset(srcwin);
	if (val) {
		if (inbounds = getloc(&loc)) {
			begsel(&loc);
			qdevice(MOUSEX);
			qdevice(MOUSEY);
			return(1);
		} 
	} else /*if (inbounds)*/ {
		getloc(&loc);
		endsel(&loc);
		unqdevice(MOUSEX);
		unqdevice(MOUSEY);
		selecting = 0;
		return(0);
	}
}

do_contsel()
{
	locate	loc;
	if (!qtest() && getloc(&loc)) {
		contsel(&loc);
	}
}

    	/* default point selection before 1st character */
colrow tmark = {0, 0};		/* quiet end of current selection */
colrow tpoint = {0, 0};		/* gap highlighted end of current selection */
colrow tinsert = {0,0};	/* beginning of inserted chars, for later selection */
colrow tcur;			/* active end of extending selection */
int begselpending;

    /*
     * begin selection from pointing device; button comes down
     */
begsel(loc)
    locate *loc;
{
    begselpending = TRUE;
    if (hitcolrow(loc, &tcur)) {
	begselpending = FALSE;

	plottpoint(OFF);
	selhilight(gsvp->tf, gsvp->tv, DEHILIGHT, &tmark, &tpoint);
	tmark = tcur;
	tpoint = tcur;
	plottpoint(ON);
    }
}

    /*
     * continue selection from pointing device; button stays down, dragged
     */
contsel(loc)
    locate *loc;
{
    if (begselpending)
	begsel(loc);

    if (hitcolrow(loc, &tcur)) {
	extendsel(&tcur);
    }
}

    /*
     * end selection from pointing device; button comes up
     */
endsel(loc)
    locate *loc;
{
	int	start;
	int	end;
	char	*p, *q, *find_end(), *get_selline();
	
	color_bps(gsvp);
	if (gsvp->curline.mark.row == tmark.row) {
		setcolor(gsvp->tf, gsvp->tv, GREENBG, &(gsvp->curline.mark), 
			&(gsvp->curline.point));
	}
	selhilight(gsvp->tf, gsvp->tv, HILIGHT, &tmark, &tcur);
    if (begselpending)
	begsel(loc);

    if (hitcolrow(loc, &tcur)) {
	extendsel(&tcur);
    }
    tinsert = tpoint;
}

    /*
     * convert locator input to textcoordinate
     */
    int
hitcolrow(loc, tc)
    locate *loc;
    colrow *tc;
{
    int r;

    if (r = tvpixtopos(gsvp->tv, loc->x, loc->y, &tc->row, &tc->col)) {
	if (r & OUTOF_TV)
	    return 0;
	if (r & COL_ABOVE_TF)	  /* column off end of row;           */
	    stepforward(gsvp->tf, tc);  /* advance to beginning of next row */
    }
    if (tc->col && (tc->col == tfnumcols(gsvp->tf, tc->row)) &&
		   (tfgetchar(gsvp->tf, tc->row, tc->col-1) == '\n'))
	tc->col--;
    return 1;
}


    /*
     *  step the colrow to be moved to the next successive colrow address
     *    return TRUE if colrow has moved to a different row
     */
    int
stepforward(tf, moved)
    register textframe *tf;
    register colrow *moved;
{
    long nr;

    if (++(moved->col) > lastcolumn(moved->row)) {
	if (++(moved->row) > (nr = (tfnumrows(tf) - 1))) {
	    moved->row = nr;
	    moved->col = lastcolumn(moved->row);
	}
	else {
	    moved->col = 0;
	    return TRUE;
	}
    }
    return FALSE;
}


    /*
     *  return column position in row at xpix, limited by max column position
     */
    long
lastcolumn(row)
    long row;
{
    long col;
    
    if ((col = tfnumcols(gsvp->tf, row)) && (tfgetchar(gsvp->tf, row, col-1) == '\n'))
	col--;

    return col;
}
extendsel(loc)
    locate *loc;
{
    static int prevseldirection = 0;
    int seldirection, extdirection;

#ifdef DEBUG
    Dbg(0, tracesel("extend select tcur", loc, &tcur));
#endif

    if (extdirection = dir(&tpoint, &tcur)) {
	seldirection = dir(&tmark, &tcur);
	plottpoint(OFF);
	if (seldirection == prevseldirection) {
	    if (extdirection != seldirection)
		 selhilight(gsvp->tf, gsvp->tv, DEHILIGHT, &tpoint, &tcur);
	    else selhilight(gsvp->tf, gsvp->tv, HILIGHT, &tpoint, &tcur);
	}
	else {
	    selhilight(gsvp->tf, gsvp->tv, DEHILIGHT, &tpoint, &tmark);
	    selhilight(gsvp->tf, gsvp->tv, HILIGHT, &tmark, &tcur);
	    prevseldirection = seldirection;
	}
	tpoint = tcur;
	plottpoint(ON);
    }
}

plottpoint(status)
    int status;
{
    long x, y, rowht, clr;

    if (status == ON)
	 clr = BLACK;
    else clr = WHITE;

    tvpostopix(gsvp->tv, tpoint.row , tpoint.col, &x, &y);
    rowht = tvrowheight(gsvp->tv, tpoint.row);
    color(clr);
    rectfi(x-1, y, x, y+rowht-1);
}

#define sgn(a) (((a)==0) ? 0 : (((a)<0) ? -1 : 1))

dir(old, new)
    colrow *old, *new;
{
    int dir;

    dir = sgn(new->row - old->row);
    if (dir == 0)
	dir = sgn(new->col - old->col);
    return dir;
}

int
getloc(loc)
    register locate *loc;
{
    int xclamped, yclamped;

    loc->x = clamp(getvaluator(CURSORX)-xorg, 0, xsize-1, &xclamped);
    loc->y = clamp(getvaluator(CURSORY)-yorg, 0, ysize-1, &yclamped);

    return (loc->inbounds = !(xclamped || yclamped));
}


clamp(value, minvalue, maxvalue, clamped)
    register int value, minvalue, maxvalue;
    register int *clamped;
{
    *clamped = TRUE;
    if (value < minvalue)
	return minvalue;
    if (value > maxvalue)
	return maxvalue;

    *clamped = FALSE;
    return value;
}
char	sel_strbuf[1096];

char	*
get_selstr()
{
	int	length;

	tfsetmark(gsvp->tf, tmark.row, tmark.col);
	tfsetpoint(gsvp->tf, tcur.row, tcur.col);
	length = tfgetascii(gsvp->tf, sel_strbuf, 1096);
	sel_strbuf[length] = '\0';
	return(&(sel_strbuf[0]));
}

char	*
get_selline() {
	int	length;
	char	*p;

	tfsetmark(gsvp->tf, tmark.row, 0);
	tfsetpoint(gsvp->tf, tcur.row, 1096);
	length = tfgetascii(gsvp->tf, sel_strbuf, 1096);
	sel_strbuf[length] = '\0';
	p = (char *) malloc(length + 1);
	strcpy(p, sel_strbuf);
	return(p);
}

int
get_sellineno() {
	return(tmark.row + 1);
}

char	*
get_sel_string() {
	char	*linep;
	char	*stringp;
	char	*p;
	int	start;
	int	end;


	linep = get_selline();
	if (tmark.row == tcur.row && tmark.col <= tcur.col) {
		stringp = find_end(linep, tmark.col, tcur.col, &start, &end);
		stringp[end - start] = '\0';
	} else if (tmark.row == tcur.row) {
		stringp = find_end(linep, tcur.col, tmark.col, &start, &end);
		stringp[end - start] = '\0';
	} else if (tmark.col < tcur.col) {
		stringp = linep + tmark.col;
		for (p = stringp; *p != '\n'; p++);
		*p = '\0';
	} else {
		stringp = linep + tmark.col;
		for (p = stringp; *p != '\n'; p++);
		*p = '\0';
	}
		
	return(stringp);
}

int
src_has_changed(filename)
char	*filename;
{
	SRCVIEW	*svp;
	struct	stat	statbuf;

	stat(filename, &statbuf);
	svp = findsrcview(filename);
	if (svp->date < statbuf.st_mtime) {
		tffree(svp->tf);
		tvfree(svp->tv);
		svp->tf = (textframe *) 0;
		init_srcwin(svp->filename);
		return(1);
	} else {
		return(0);
	}
}
