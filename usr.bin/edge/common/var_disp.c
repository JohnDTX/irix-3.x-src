

#include <gl.h>
#include <device.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "window.h"
#include "tf.h"
#include "manage.h"
extern	short	charwidth, charheight, chardescender;
int	selecting;
static	int	xorg, yorg;
SIZE_DESC	var_rect = { 6, 143, 80, 22, 1, 1, 1 };

#define CHANGED 0
#define HILIGHT   0
#define DEHILIGHT 1
#define MARGINSZ 20
#define REDBG (RED << LOOKS_BGSHIFT)
#define GREENBG (GREEN << LOOKS_BGSHIFT)
#define DEF_BG (dpagecolor << LOOKS_BGSHIFT)

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

typedef	struct	display_var {
	struct	display_var	*nextdv;
	char	*var_name;
	int	nlines;
	int	numbytes;
	SELECTION	var_loc;
	int	topline;
} DISPLAY_VAR;

#define HASHSIZE 100
typedef	struct varview {
	int	window_id;
	textframe	*tf;
	textview	*tv;
	int	xsize;
	int	ysize;
	int	xorg;
	int	yorg;
	long	looks;
	long	topline;
	long	nlines;
	long	viewlines;
	long	bottomline;
	DISPLAY_VAR	*display_vars[HASHSIZE];
} VARVIEW;



VARVIEW	var_view;
VARVIEW	*vvp;
int	var_win = -1;


init_varwin()
{
	int	*p;
	int	i;

	vvp = &var_view;
	vvp->tf = tfnew();
	tfsetlooks(vvp->tf, (0 << LOOKS_FONTSHIFT) |
		   (dtextcolor << LOOKS_FGSHIFT) |
		   (dpagecolor << LOOKS_BGSHIFT));

	vvp->looks = mytfgetlooks(vvp->tf);

#ifdef notdef
	if (flag_font) {
		tffont(svp->tf, 1);
	}
#endif

#ifdef notdef
	reset_constraints(WT_SRC);
#endif
	stepunit(charheight, charwidth);
	var_win = vvp->window_id = winopen("variables");
	winset(var_win);
	winconstraints();
	stepunit(charheight, charwidth);
	winconstraints();
	getorigin(&xorg, &yorg);
	wintitle("edge: Variable Display Window");
	vvp->tv = tvnew(vvp->tf);
	vvp->bottomline = 1;
	tvleftpix(vvp->tv, MARGINSZ);
	getsize(&vvp->xsize, &vvp->ysize);
	getorigin(&vvp->xorg, &vvp->yorg);
	vvp->nlines = 0;
#ifdef notdef
	if (flag_font) {
		tvmapfont(vvp->tv, 1, flag_font);
	}
#endif
	vvp->viewlines = (vvp->ysize / charheight) - 1;
	tvviewsize(vvp->tv, vvp->xsize, vvp->ysize);
	tvsetbg(vvp->tv, dpagecolor);
	tvsetstops(vvp->tv, 8*8, 0);
	tvtoprow(vvp->tv,0);
	tvhighlight(vvp->tv, CYAN, 0);
	color_bps(vvp);
	tvdraw(vvp->tv,1);
}


var_display(string)
char	*string;
{
	long	looks;
	DISPLAY_VAR	*dvp;
	DISPLAY_VAR	dv;
	char	*line;
	char	*strtok();
	DISPLAY_VAR	*add_var();
	DISPLAY_VAR	*find_display_var();
    textcoord begin;

	dvp = &dv;
	winset(var_win);
/*
	if ((dvp = find_display_var(var_name)) == 0) {
		dvp = add_var(var_name, string);
	} else {
		dvp->nlines = get_line_bytes(string, &dvp->numbytes);
	}
	tfsetpoint(vvp->tf, dvp->var_loc.point.row, 
		dvp->var_loc.point.col);
	tfsetmark(vvp->tf, dvp->var_loc.mark.row + dvp->nlines,
		200);
	tfdelete(vvp->tf);
	dvp->var_loc.mark.col = dvp->numbytes;
	tfsetpoint(vvp->tf, dvp->var_loc.point.row, 
		dvp->var_loc.point.col);
	tfsetmark(vvp->tf, dvp->var_loc.mark.row, dvp->var_loc.mark.col);
*/
	line = strtok(string, "\n");
    tfgetpoint(vvp->tf, &begin.tc_row, &begin.tc_col);
	tfputascii(vvp->tf, line, strlen(line));
	tfsplit(vvp->tf);
	vvp->nlines++;
	while ((line = strtok(NULL, "\n")) != NULL) {
		tfputascii(vvp->tf, line, strlen(line));
		tfsplit(vvp->tf);
		vvp->nlines += 1;
	}
	if (vvp->nlines > vvp->viewlines) {
		tvtoprow(vvp->tv, vvp->nlines - vvp->viewlines);
	}
    tfsetmark(vvp->tf, begin.tc_row, begin.tc_col);
		

/*
fprintf(stderr, "var_display:STRING:%d:%s\n", dvp->nlines, string);
fprintf(stderr, "var_display:variable name = %s mark=%d %d point=%d %d\n", var_name, dvp->var_loc.mark.row, dvp->var_loc.mark.col, dvp->var_loc.point.row, dvp->var_loc.point.col);
*/
	drawtext(var_win, vvp->tv, 1);
}

DISPLAY_VAR	*
add_var(var_name, string)
char	*var_name;
char	*string;
{
	int	hashval;
	DISPLAY_VAR	*dp;
	DISPLAY_VAR	*dvp;
	DISPLAY_VAR	*lastdvp;

	
	hashval = hash(var_name);
	lastdvp = (DISPLAY_VAR *) 0;
	for (dvp = vvp->display_vars[hashval]; dvp != (DISPLAY_VAR *) 0;
		dvp = dvp->nextdv) {
		lastdvp = dvp;
	}
	if (lastdvp == (DISPLAY_VAR *) 0) {
		vvp->display_vars[hashval] = 
			(DISPLAY_VAR *) malloc(sizeof(DISPLAY_VAR));
		dp = vvp->display_vars[hashval];
	} else {
		lastdvp->nextdv = (DISPLAY_VAR *) malloc(sizeof(DISPLAY_VAR));
		dp = lastdvp->nextdv;
	}
	dp->var_name = (char *) malloc(strlen(var_name) + 1);
	strcpy(dp->var_name, var_name);
	dp->nlines = get_line_bytes(string, &(dp->numbytes));
	dp->var_loc.point.row = vvp->bottomline;
	dp->var_loc.point.col = 0;
	dp->var_loc.mark.row = vvp->bottomline;
	dp->var_loc.mark.col = dp->numbytes;
	vvp->bottomline += (dp->nlines == 0)?1:dp->nlines;
	return(dp);
	
}

int
get_line_bytes(string, nbyteptr)
int	*nbyteptr;
char	*string;
{
	int	numlines = 0;
	char	*p;
	int	i = 0;

	for (p = string; *p != '\0'; p++) {
		if (*p == '\n') {
			numlines++;
		}
		i++;
	}
	*nbyteptr = i;
	return(numlines);
}
	
/*
 * Find the display_var associated with variablw name
 */
DISPLAY_VAR	*
find_display_var(var_name)
char	*var_name;
{
	int	hashval;
	DISPLAY_VAR	*dvp;

	hashval = hash(var_name);
	dvp = vvp->display_vars[hashval];
	while (dvp != (DISPLAY_VAR *) 0) {
		if (strcmp(dvp->var_name, var_name) == 0) {
			return(dvp);
		}
		dvp = dvp->nextdv;
	}
	return((DISPLAY_VAR *) 0);
}

varredisp() {
	int	xsize, ysize;
	int	cur_win;

	winset(var_win);
	getsize(&vvp->xsize, &vvp->ysize);
	getorigin(&vvp->xorg, &vvp->yorg);
	vvp->looks = mytfgetlooks(vvp->tf);
	color((vvp->looks & LOOKS_BG) >> LOOKS_BGSHIFT);
	vvp->viewlines = (vvp->ysize / charheight) - 1;
	tvviewsize(vvp->tv, vvp->xsize, vvp->ysize);
	drawtext(var_win, vvp->tv, 1);
}

reset_vars()
{
	DISPLAY_VAR	**hashtab;
	int	i;

	hashtab = (vvp->display_vars);
	for (i = 0; i < HASHSIZE; i++) {
		if (hashtab[i] != (DISPLAY_VAR *) 0) {
			do_sprint(hashtab[i]->var_name);
		}
	}
}

do_sprint(var_name)
char	*var_name;
{

	char	*sprintbuf[512];

	sprintf(sprintbuf, "sprint %s\n", var_name);
	send_shell(dbx_win, sprintbuf, strlen(sprintbuf));
}

scrollvar(val)
int	val;
{
	float	pct;

	pct = ((float) (vvp->ysize) - (float) (val - vvp->yorg)) / (float) (vvp->ysize);
	vvp->topline = tfnumrows(vvp->tf) * pct;
	tvtoprow(vvp->tv, vvp->topline);
	tvdraw(vvp->tv, 0);
}
