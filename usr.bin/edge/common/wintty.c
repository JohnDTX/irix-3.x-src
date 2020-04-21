/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/wintty.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:13 $
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/pty_ioctl.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <errno.h>
#include "window.h"
#include <gl.h>
#include "device.h"
#include "tf.h"
#include "manage.h"
#include "buttons.h"

static	int	cur_txnum = 0;
static	int	first_window = 1;
int	dpagecolor = WHITE;
int	dtextcolor = BLACK;
extern	short	charwidth, charheight, chardescender;
extern	float	max_width, max_height;
WINTTYMAP	*last_link;


SIZE_DESC	dbx_rect = { 6, 506, 80, 15, 1, 1, 1 };
SIZE_DESC	usr_rect = { 6, 3, 80, 7, 1, 1, 1 };

WINTTYMAP *
open_wintty(name, win_type)
char	*name;
char	win_type;
{

	WINTTYMAP	*wtmp;
	char	*malloc();
	int	gid;
	long	xorg, yorg;
	long	xlen, ylen;
	char	title_buf[512];


	if (dbflag) {
		foreground();
    }
	wtmp = (WINTTYMAP *) malloc(sizeof(WINTTYMAP));
	if (last_link != (WINTTYMAP *) 0) {
		last_link->nextwintty = wtmp;
		wtmp->lastwintty = last_link;
		last_link = wtmp;
	} else {
		wtmp->lastwintty = (WINTTYMAP *) 0;
		last_link = wtmp;
	}
	wtmp->nextwintty = (WINTTYMAP *) 0;
	wtmp->wt_type = win_type;
	if (first_window) {
#ifndef mips
		noport();
		gid = winopen(name);
#endif
#ifdef notdef
		if (flag_font) {
			setup_font();
		} else {
#endif
			gl_getcharinfo(&charwidth, &charheight, &chardescender);
#ifdef notdef
		}
#endif
		max_width = (MAXCOLS * charwidth) + XBORDER;
		max_height = (MAXROWS * charheight) + YBORDER;
	} 
	reset_constraints(win_type);
	wtmp->wt_gid = winopen(name);
	winset(wtmp->wt_gid);

	winconstraints();
	stepunit(charwidth, charheight);
	winconstraints();
	if (first_window) {
#ifndef mips
		winclose(gid);
#endif
		first_window = 0;
	}
#ifdef TF
	wtmp->wt_tf = tfnew();
	tfsetlooks(wtmp->wt_tf, (0 << LOOKS_FONTSHIFT) |
		   (0 << LOOKS_FGSHIFT) |
		   (10 << LOOKS_BGSHIFT));

	if (flag_font != (char *) 0) {
		tffont(wtmp->wt_tf, 1);
	}
#else
	wtmp->wt_textnum = cur_txnum;
	tx_open(cur_txnum);
	txport[cur_txnum].tx_pagecolor = dpagecolor;
	txport[cur_txnum].tx_textcolor = dtextcolor;
	cur_txnum++;
#endif

	wtmp->wt_ptynum = open_pty(&(wtmp->wt_slavename),
		&(wtmp->wt_mastername),
		&(wtmp->wt_masterfd), O_NDELAY);
#ifdef TF
	wtmp->wt_tv = tvnew(wtmp->wt_tf);
#endif
	if (win_type == WT_DBX) {
		sprintf(title_buf, "%s: Command Window", name);
	} else {
		sprintf(title_buf, "edge: User Window: %s", name);
	}
		
	wintitle(title_buf);
	cur_win = wtmp;
	winset(wtmp->wt_gid);
	getsize(&xlen, &ylen);
#ifdef TF
	if (flag_font != (char *) 0) {
		tvmapfont(wtmp->wt_tv, 1, flag_font);
	}
	if (win_type == WT_DBX) {
		tvviewsize(wtmp->wt_tv, xlen - 100, ylen);
	} else {
		tvviewsize(wtmp->wt_tv, xlen , ylen);
	}
	tvsetbg(wtmp->wt_tv, 10);
	tvsetstops(wtmp->wt_tv, 8*8, 0);
	tvtoprow(wtmp->wt_tv,0);
	tvhighlight(wtmp->wt_tv, 9, 0);
	tvdraw(wtmp->wt_tv,1);
#else
#endif
	wtmp->position.xlen = xlen;
	wtmp->position.ylen = ylen;
	getorigin(&xorg, &yorg);
	wtmp->position.xorg = (short) xorg;
	wtmp->position.yorg = (short) yorg;
	wtmp->stopped = 0;
	wtmp->realcursorcolor = GREEN;
	wtmp->need_read = 0;
	wtmp->need_update = 0;
	wtmp->need_redisplay = 0;
	/*
	wtmp->read_bytes = 0;
	*/
	txport[wtmp->wt_textnum].tx_cursorcolor = GREEN;
	if (win_type == WT_DBX) {
		dbx_win = wtmp;
		dbxredisp();
	} else {
		redisplay(wtmp);
	}
	return(wtmp);
}


int
open_pty(slave_name, master_name, master_fd, dodelay)
char	**slave_name;
char	**master_name;
int	*master_fd;
int	dodelay;
{
	int	pty_num;
	char	*malloc();
	struct	stat	sb;

	/*
	 * First, get a pseudo-tty to use.
	 */
	*slave_name = malloc(PTYNMSIZE);
	*master_name = malloc(PTYNMSIZE);



	/*
	 * Open clone driver for pty and get a controller
	 */
	if ((*master_fd = open("/dev/ptc", O_RDWR|dodelay)) >= 0) {
		if (fstat(*master_fd, &sb) < 0) {
			perror("gsh: can't stat");
			myexit(-1);
		}
		pty_num = minor(sb.st_rdev);
		sprintf(*slave_name, "/dev/ttyq%d", pty_num);
		sprintf(*master_name, "/dev/ptc%d", pty_num);
		if (ioctl(*master_fd, PTIOC_QUEUE, 0) == -1) {
			perror("edge");
			fprintf(stderr, "cannot set %s to PTIOC_QUEUE\n",
				*master_name);
			myexit(-1);
		}
		goto skip;
	}
skip:
		return(pty_num);
}

win_exec(wtmp, argv)
WINTTYMAP	*wtmp;
char	**argv;
{
}

int
win_read(wtmp, buf, nbytes) 
WINTTYMAP	*wtmp;
char	*buf;
int	nbytes;
{
	int nb;
	short t, v;
	int	i;

	wtmp->need_read = 0;			/* we are doing it now! */
	nb = read(wtmp->wt_masterfd, buf, nbytes);
	(void) ioctl(wtmp->wt_masterfd, PTIOC_QUEUE, 0);
	if (nb < 0) {
		if (errno == EAGAIN) {
			return;
		}
		byebye();
	}
	wtmp->read_bytes += nb;
	return(nb);

}

win_write(wtmp, buffer, nbytes)
WINTTYMAP	*wtmp;
char	*buffer;
int	nbytes;
{
	int	cur_gid;

	cur_gid = winget();

	winset(wtmp->wt_gid);
	tx_addchars(wtmp->wt_textnum, buffer, nbytes);
	winset(cur_gid);
}
	

/*
 * Setup window constraints
 */
reset_constraints(wintype)
char	wintype;
{
	SIZE_DESC size_loc;

	if (wintype == WT_DBX) {
		size_loc = dbx_rect;
		while (((size_loc.rect.ylen + 1) * charheight) 
			< (NSIDEBUTTS * 20)) {
			size_loc.rect.ylen += 1;
		}
		if (size_loc.is_default == 1) {
			size_loc.rect.yorg =  
				((src_rect.rect.ylen + 1) 
				* charheight) +1 + 17 + src_rect.rect.yorg;
		}
	} else if (wintype == WT_USER) {
		size_loc = usr_rect;
	} else if (wintype == WT_SRC) {
		size_loc = src_rect;
		if (size_loc.is_default == 1) {
			size_loc.rect.yorg =  
				((usr_rect.rect.ylen + 1) 
				* charheight) +1 + 17 + usr_rect.rect.yorg;
		}
	}
	if (size_loc.hassize == 1 && size_loc.hasorig == 1) {
		prefposition((int) size_loc.rect.xorg, 
			size_loc.rect.xorg + ((size_loc.rect.xlen) * charwidth),
			size_loc.rect.yorg, 
			size_loc.rect.yorg + ((1 + size_loc.rect.ylen) * 
				charheight) /*- 1*/);
	} else if (size_loc.hassize == 1) {
		prefsize((size_loc.rect.xlen * charheight),
			(size_loc.rect.ylen * charwidth));
	} else {
		minsize(charwidth + XBORDER, charheight + YBORDER);
		maxsize((int) max_width, (int) max_height);
	}
	stepunit(charwidth, charheight);
}

getwinsize(i, gid)
int	gid;
int	i;
{

	int	x;
	int	y;
	int	xorg;
	int	yorg;
	int 	win_save;

	
	win_save = winget();
	winset(gid);

	
	getsize(&x, &y);
	getorigin(&xorg, &yorg);
	fprintf(stderr, "%d:x: %d y: %d org=%d,%d\n", i, x/charwidth, y/charheight, xorg, yorg);
	winset(win_save);
}


free_wintty(wtmp)
WINTTYMAP	*wtmp;
{

	wtmp->lastwintty->nextwintty = wtmp->nextwintty;
	if (wtmp->nextwintty != (WINTTYMAP *) 0) {
		wtmp->nextwintty->lastwintty = wtmp->lastwintty;
	} else if (wtmp == last_link) {
		last_link = wtmp->lastwintty;
	}
	free(wtmp);
}



WINTTYMAP	*
ptynum_to_window(v) 
int	v;
{
	WINTTYMAP	*wp;

	for (wp = dbx_win; wp != (WINTTYMAP *) 0; wp = wp->nextwintty) {
		if (wp->wt_ptynum == v) {
			return(wp);
		}
	}
	return((WINTTYMAP *) 0);
}

WINTTYMAP	*
pid_to_wintty(v) 
int	v;
{
	WINTTYMAP	*wp;

	for (wp = dbx_win; wp != (WINTTYMAP *) 0; wp = wp->nextwintty) {
		if (wp->wt_pid == v) {
			return(wp);
		}
	}
	return((WINTTYMAP *) 0);
}



WINTTYMAP	*
gid_to_wintty(v) 
int	v;
{

	WINTTYMAP	*wp;
	
	for (wp = dbx_win; wp != (WINTTYMAP *) 0; wp = wp->nextwintty) {
		if (wp->wt_gid == v) {
			return(wp);
		}
	}
	return((WINTTYMAP *) 0);
}


disc_winttys()
{

	WINTTYMAP	*wp;

	for (wp = dbx_win; wp != (WINTTYMAP *) 0; wp = wp->nextwintty) {
		wp->disconnected = 1;
		wp->need_update = 1;
		txport[wp->wt_textnum].tx_state &= ~TX_SELECTED;
		qenter(REDRAW, wp->wt_gid);
	}
}


connect_wintty(wtmp)
WINTTYMAP	*wtmp;
{
	WINTTYMAP	*wp;

	for (wp = dbx_win; wp != (WINTTYMAP *) 0; wp = wp->nextwintty) {
		if (wp != wtmp) {
			wp->disconnected = 1;
			txport[wp->wt_textnum].tx_state &= ~TX_SELECTED;
			wp->need_update = 1;
			qenter(REDRAW, wp->wt_gid);
		} else {
			txport[wp->wt_textnum].tx_state |= TX_SELECTED;
			wp->disconnected = 0;
			wp->need_update = 1;
			winset(wp->wt_gid);
			kb_fix();
		}
	}
}



setsize(linep)
char	*linep;
{
	char	*window, *asc_xlen, *asc_ylen;
	SIZE_DESC	*size_locp;
	char	*strtok();

	window = strtok(NULL, " \t\n");
	if (strcmp(window, "dbx") == 0) {
		size_locp = &dbx_rect;
	} else if (strcmp(window, "user") == 0)  {
		size_locp = &usr_rect;
	} else if (strcmp(window, "source") == 0)  {
		size_locp = &src_rect;
	}
	if ((asc_xlen = strtok(NULL, " \t\n")) == (char *) 0) {
		size_locp->hassize = 0;
	} else {
		asc_ylen = strtok(NULL, " \t\n");
		size_locp->rect.xlen = atoi(asc_xlen);
		size_locp->rect.ylen = atoi(asc_ylen);
	}
}


setorigin(linep)
char	*linep;
{
	char	*window, *asc_xorg, *asc_yorg;
	SIZE_DESC	*size_locp;
	char	*strtok();

	window = strtok(NULL, " \t\n");
	if (strcmp(window, "dbx") == 0) {
		size_locp = &dbx_rect;
	} else if (strcmp(window, "user") == 0)  {
		size_locp = &usr_rect;
	} else if (strcmp(window, "source") == 0)  {
		size_locp = &src_rect;
	}
	if ((asc_xorg = strtok(NULL, " \t\n")) == (char *) 0) {
		size_locp->hasorig = 0;
	} else {
		asc_yorg = strtok(NULL, " \t\n");
		size_locp->rect.xorg = atoi(asc_xorg);
		size_locp->rect.yorg = atoi(asc_yorg);
	}
}


setdcolor(linep)
char	*linep;
{
	char	*bg;
	char	*fg;

	fg = strtok(NULL, " \t\n");
	bg = strtok(NULL, " \t\n");
	dpagecolor = atoi(bg);
	dtextcolor = atoi(fg);
	
}
