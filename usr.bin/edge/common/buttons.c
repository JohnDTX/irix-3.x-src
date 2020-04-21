/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/buttons.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:47 $
 */

/*
 * This procedure runs the shell...
 */
#include "errno.h"
#include "sys/pty_ioctl.h"
#include "sys/termio.h"
#include "stdio.h"
#include "gl.h"
#include "device.h"
#include "window.h"
#include "gsh.h"
#include "kb.h"
#include "tf.h"
#include "buttons.h"
#include "manage.h"
#include "dbxshm.h"

BUTT_T	sidebutts[NSIDEBUTTS] = {
	0, 0, 100, 20, "  rerun", runuser , 4, 7,
	0, 0, 100, 20, "  stop", dostop, 4, 7,
	0, 0, 100, 20, "  step", dostep, 4, 7,
	0, 0, 100, 20, "  next", donext, 4, 7,
	0, 0, 100, 20, "  cont", docont, 4, 7,
	0, 0, 100, 20, "  print", doprint, 4, 7,
	0, 0, 100, 20, "  where", dowhere, 4, 7,
	0, 0, 100, 20, "  trace", dotrace,  4, 7,
	0, 0, 100, 20, "  interrupt", dointerrupt,  4, 7,
	0, 0, 100, 20, "  sh", dosh, 4, 7,
	0, 0, 100, 20, "  quit", doquit, 4, 7,
};

/*
BUTT_T	bottombutts[NBOTTOMS] = {
	0, 0, 0, 20, "make1", domake,
	0, 0, 0, 20, "make2", domake,
	0, 0, 0, 20, "make3", domake,
	0, 0, 0, 20, "make4", domake,
	0, 0, 0, 20, "make5", domake
};
*/

extern	int	master_fd;
extern	int	script_fd;
WINTTYMAP	*shell_win;

char	need_read, need_update;
char	stopped, disconnected;
int	savedcursorcolor, realcursorcolor;
int	read_bytes;
char	need_redisplay;
char	cursor_change;
WINTTYMAP	*cur_win;


leftmouse(mousex, mousey)
short	mousex;
short	mousey;
{

	short	win_x;
	short	win_y;

	win_x = mousex - dbx_win->position.xorg;
	win_y = mousey - dbx_win->position.yorg;
	call_button(win_x, win_y);
}


call_button(x, y)
short	x;
short	y;
{
	rect_t	rectangle;
	int	i;

	rectangle = sidebutts[0].b_position;
	if (x > rectangle.xorg && x < rectangle.xorg + rectangle.xlen) { 
		for (i = 0; i < NSIDEBUTTS; i++) {
			rectangle = sidebutts[i].b_position;
			if ((y >= rectangle.yorg) && 
				(y <= (rectangle.yorg + rectangle.ylen))) {
				inv_button(&(sidebutts[i]));
				(*sidebutts[i].b_func)();
				draw_button(&(sidebutts[i]));
				return;
			}
		}
	}
}

runuser()
{
	char	runbuf[100];

	sprintf(runbuf, "rerun\n");
	send_shell(dbx_win, runbuf, strlen(runbuf));
}


docont()
{
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "cont\n", strlen("cont\n"));
}

dostep()
{
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "step\n", strlen("step\n"));
}

donext()
{
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "next\n", strlen("next\n"));
}

dolist()
{
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "list\n", strlen("list\n"));
}

dowhere()
{
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "where\n", strlen("where\n"));
}

dotrace()
{
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "trace\n", strlen("trace\n"));
}

redraw_buttons(rectp)
rect_t	*rectp;
{
	
	redraw_sidebutts(rectp);
}

redraw_sidebutts(rectp)
rect_t	*rectp;
{

	int	i;
	BUTT_T	*buttp;

#ifdef TF
	color(getbg(dbx_win->wt_tf));
#else
	color(txport[dbx_win->wt_textnum].tx_pagecolor);
#endif
	rectfi(rectp->xlen - rectp->xorg - 100, 0, rectp->xlen, rectp->ylen);
	for (i = 1; i <= NSIDEBUTTS; i++) {
		buttp = &sidebutts[i-1];
		buttp->b_position.xorg = rectp->xlen - 100;
		buttp->b_position.yorg = rectp->ylen - ((i*20));
		draw_button(buttp);
	}
/*
	color(buttp->charcolor);
	move2s(sidebutts[0].b_position.xorg, sidebutts[0].b_position.yorg +
		sidebutts[0].b_position.ylen);
	draw2s(sidebutts[NSIDEBUTTS-1].b_position.xorg, 
		sidebutts[NSIDEBUTTS-1].b_position.yorg);
*/
		
}

inv_button(buttp)
BUTT_T	*buttp;
{
	rect_t	r;
	int	xlen, ylen;
	int	cur_win;

	cur_win = winget();

	winset(dbx_win->wt_gid);
	getsize(&xlen, &ylen);
	r.yorg = 0;
	r.xorg = 3;
	r.xlen = xlen;
	r.ylen = ylen;
	setoutput(&r);
	color(buttp->charcolor);
	rectfi(buttp->b_position.xorg, buttp->b_position.yorg,
		buttp->b_position.xorg + buttp->b_position.xlen,
		buttp->b_position.yorg + buttp->b_position.ylen);
	color(buttp->backcolor);
	cmov2i(buttp->b_position.xorg, buttp->b_position.yorg + 5);
	charstr(buttp->b_string);
}


draw_button(buttp)
BUTT_T	*buttp;
{
	rect_t	r;
	int	xlen, ylen;

	winset(dbx_win->wt_gid);
	getsize(&xlen, &ylen);
	r.yorg = 0;
	r.xorg = 3;
	r.xlen = xlen;
	r.ylen = ylen;
	setoutput(&r);
	color(buttp->backcolor);
	rectfi(buttp->b_position.xorg, buttp->b_position.yorg,
		buttp->b_position.xorg + buttp->b_position.xlen + 1,
		buttp->b_position.yorg + buttp->b_position.ylen - 2);
	color(buttp->charcolor);
	cmov2i(buttp->b_position.xorg-2, buttp->b_position.yorg + 5);
	charstr(buttp->b_string);
	move2s(buttp->b_position.xorg, buttp->b_position.yorg);
	draw2s(buttp->b_position.xorg + buttp->b_position.xlen,
		buttp->b_position.yorg);
	move2s(buttp->b_position.xorg, buttp->b_position.yorg);
	draw2s(buttp->b_position.xorg + buttp->b_position.xlen,
		buttp->b_position.yorg);
}

/*
redraw_bottom(rectp)
rect_t	*rectp;
{
	int	lcolor = 480;
	int	i;
	BUTT_T	*buttp;

#ifdef TF
	color(getbg(wtmp->wt_tf));
#else
	color(txport[dbx_win->wt_textnum].tx_pagecolor);
#endif
	rectfi(rectp->xorg, rectp->yorg, rectp->xlen, 100);
	for (i = 1; i <= NBOTTOMS; i++) {
		buttp = &bottombutts[i-1];
		buttp->b_position.xorg = rectp->xorg;
		buttp->b_position.yorg = rectp->yorg + (100 - (i*20));
		buttp->b_position.xlen = rectp->xlen;
		color(lcolor);
		lcolor += 2;
		rectfi(buttp->b_position.xorg, buttp->b_position.yorg,
			buttp->b_position.xorg + buttp->b_position.xlen,
			buttp->b_position.yorg + buttp->b_position.ylen - 1);
		color(0);
		cmov2i(buttp->b_position.xorg + 2, buttp->b_position.yorg + 5);
		charstr(buttp->b_string);
		move2s(buttp->b_position.xorg, buttp->b_position.yorg);
		draw2s(buttp->b_position.xorg + buttp->b_position.xlen,
			buttp->b_position.yorg);
		move2s(buttp->b_position.xorg, buttp->b_position.yorg + 1);
		draw2s(buttp->b_position.xorg + buttp->b_position.xlen,
			buttp->b_position.yorg + 1);
	}
		
}
*/


doquit() {
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "quit\n", strlen("quit\n"));
}

dostatus() {
	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	send_shell(dbx_win, "status\n", strlen("status\n"));
}

dosh()
{
	char	*argv[10];
	int	av = 0;
	int	pid;
	char	*p;

	argv[av++] = "/usr/lib/gsh";
	argv[av++] = (char *) 0;
	if ((pid = fork()) == 0) {
		shmdetach();
		execv(argv[0], argv);
		perror("child:edge");
	}
	if (pid == -1) {
		perror("parent:edge");
	}
}

dostop()
{
	char	stopbuf[512];
	char	*p;
	char	*strip_usedir();

	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	if (src_has_changed(cur_file)) {
		sprintf(stopbuf, "Source file %s has changed, Rereading %s, please try setting breakpoint again\n", cur_file, cur_file);
		win_write(dbx_win, stopbuf, strlen(stopbuf));
		update(dbx_win);
		send_shell(dbx_win, "\n", 1);
		p = strip_usedir(cur_file);
		sprintf(stopbuf, "file %s\n", p);
		return;
	}
	p = strip_usedir(cur_file);
	sprintf(stopbuf, "file %s\n", p);
	send_shell(dbx_win, stopbuf, strlen(stopbuf));
	sprintf(stopbuf, "stop at %d\n", get_sellineno());
	send_shell(dbx_win, stopbuf, strlen(stopbuf));
}

doprint() {
	char	printbuf[512];
	char	*linep;
	char	*selectp;
	char	*get_sel_string();

	cur_win = dbx_win;
	winset(dbx_win->wt_gid);
	sprintf(printbuf, "print %s\n", get_sel_string());
	send_shell(dbx_win, printbuf, strlen(printbuf));
}

dointerrupt() {
	struct	termio	term;

	ioctl(0, TCGETA, &term);
	send_shell(dbx_win, &(term.c_cc[VINTR]), 1);
}

addbutton(linep)
char	*linep;
{
fprintf(stderr, "addbutton\n");
}
