/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/shelltool.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:02 $
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

extern	int	master_fd;
extern	int	script_fd;
extern	int	srcwin;
extern	int	var_win;
int	scroll_win;
int	scrolling;
#ifdef SPEED_BAR
int	setting_speed;
#endif

char	need_read, need_update;
char	stopped, disconnected;
int	savedcursorcolor, realcursorcolor;
int	read_bytes;
char	need_redisplay;
char	cursor_change;
WINTTYMAP	*cur_win;
WINTTYMAP	*read_win;


/*
 * When we need to redisplay ourselves, this is what
 * we do
 */
void
redisplay(wtmap)
WINTTYMAP	*wtmap;
{
	int	curwin;
	int xlen, ylen;
	int xorg, yorg;
	int xlast, ylast;
	rect_t r;
	static int oxlen, oylen;
	int tmp;


	winset(wtmap->wt_gid);
	getsize(&xlen, &ylen);
	r.xlen = (short) xlen;
	r.ylen = (short) ylen;
	r.xorg = 0;
	r.yorg = 0;
	getorigin(&xorg, &yorg);
	wtmap->position.xorg = (short) xorg;
	wtmap->position.yorg = (short) yorg;
	wtmap->position.xlen = (short) xlen;
	wtmap->position.ylen = (short) ylen;
	setoutput(&r);
	color(dpagecolor);
	rectfi(r.xorg, r.yorg, xlen, ylen);
#ifdef TF
	tvviewsize(wtmap->wt_tv, xlen, ylen);
	tvdraw(wtmap->wt_tv, 1);
#else
	setoutput(&r);
	r.xorg = 3;
	tf_redraw(wtmap->wt_textnum, &(r));
#endif
	wtmap->need_redisplay = 0;
}

/*
 * Update the text display, modifying the cursor color according to
 * external state
 */
void
update(wtmp)
WINTTYMAP	*wtmp;
{
	int	curwin;

	winset(wtmp->wt_gid);
	if ((wtmp->stopped || wtmp->disconnected) && !wtmp->savedcursorcolor) {
		wtmp->realcursorcolor = txport[wtmp->wt_textnum].tx_cursorcolor;
		wtmp->savedcursorcolor = 1;
	}

	if (wtmp->stopped)
		txport[wtmp->wt_textnum].tx_cursorcolor = RED;
	if (wtmp->disconnected)
		txport[wtmp->wt_textnum].tx_cursorcolor = 
			txport[wtmp->wt_textnum].tx_pagecolor;
	if (!wtmp->stopped && !wtmp->disconnected && wtmp->savedcursorcolor) {
		txport[wtmp->wt_textnum].tx_cursorcolor = wtmp->realcursorcolor;
		wtmp->savedcursorcolor = 0;
	}

	wtmp->need_update = 0;
	tx_update(wtmp->wt_textnum);
}

/*
 * Send a keyboard character to the shell.  What we recieve here is a
 * raw keyboard button press.  Translate the raw button into a given
 * ascii key code according to a lookup function.
 */
void
write_shell(wtmp, stroke, key)
	WINTTYMAP	*wtmp;
	int stroke, key;
{
	extern int master_fd;
	register int i, n;
	register short *k;
	short keybuf[500];
	char c;

	n = kb_translate(stroke, key, keybuf);
	if (n) {
		k = &keybuf[0];
		for (i = n; i--; k++) {
			if (*k & 0xFF00) {
				if (*k == CODE_BREAK)
					ioctl(wtmp->wt_masterfd, TCSBRK, 0);
			} else {
				c = *k;
				if (write(wtmp->wt_masterfd, &c, 1) < 0) {
					byebye();
				}
			}
		}
	}
}

/*
 * Handle a command from the graphics queue
 */
int	last_win;
void
queue_cmd(t, v)
	short t, v;
{
	short	val;
	short	mousex, mousey;
	int	cur_gid;
	WINTTYMAP	*ptynum_to_window();
	WINTTYMAP	*pid_to_wintty();
	WINTTYMAP	*gid_to_wintty();
	WINTTYMAP	*deadwin;

	/* check for keyboard buttons */
	if (t <= MAXKBDBUT) {
		write_shell(cur_win, v, t);
		return;
	}

	/* handle the rest of the events */
/*
	if (dbflag) {
		fprintf(stderr, "EVENT:%d\n", t);
	}
*/
	switch (t) {
	  case TIMER0:
		unqdevice(TIMER0);
		if (cur_win->read_bytes || cur_win->need_update) {
			cur_win->read_bytes = 0;
			update(cur_win);
		}
		break;
	  case REDRAW:
		if (v == srcwin) {
			srcredisp();
#ifdef SPEED_BAR
		} else if (v == speed_win) {
			speedredisp();
#endif
		} else if (v == var_win) {
			varredisp();
		} else if (dbx_win == gid_to_wintty(v)) {
			dbxredisp();
		} else if (gid_to_wintty(v)) {
			redisplay(gid_to_wintty(v));
		}
		break;
	  case INPUTCHANGE:
		if (v == 0) {
			disc_winttys();
		} else if ((v != srcwin) 
#ifdef SPEED_BAR
			&& (v != speed_win) 
#endif
			&& (v != var_win)) {
			WINTTYMAP	*wp;

			if ((wp = gid_to_wintty(v)) != (WINTTYMAP *) 0) { 
				cur_win = wp;
				connect_wintty(cur_win);
			}
		}
		break;
	  case MENUBUTTON:
		val = qread(&mousex);
		val = qread(&mousey);
		if (v) {
			domenu(mousex, mousey);
		}
		break;
	  case MIDDLEMOUSE:
		if (v) {
			val = qread(&mousex);
			val = qread(&mousey);
#ifdef mips
			scroll_win = gl_winat(mousex, mousey);
#else
			scroll_win = winat(mousex, mousey);
#endif
			if ((scroll_win == srcwin) || (scroll_win == var_win)) {
				scrolling = 1;
				qdevice(MOUSEY);
				qdevice(MOUSEX);
			} else {
				scroll_win = -1;
			}
		} else {
			scroll_win = -1;
			scrolling = 0;
			unqdevice(MOUSEY);
			unqdevice(MOUSEX);
		}
		break;

	  case MOUSEX:
	  case MOUSEY:
#ifdef SPEED_BAR
		if (setting_speed && (t == MOUSEY)) {
			set_speed(v);
		} else 
#endif
		if (scrolling && (t == MOUSEY)) {
			scrollwin(scroll_win, v);
		} else if (selecting) {
			do_contsel();
		}
		break;

	  case LEFTMOUSE:
		val = qread(&mousex);
		val = qread(&mousey);
#ifdef mips
		cur_gid = gl_winat(mousex, mousey);
#else
		cur_gid = winat(mousex, mousey);
#endif
#ifdef SPEED_BAR
		if (cur_gid == speed_win) {
			if (v) {
				setting_speed = 1;
				qdevice(MOUSEY);
				qdevice(MOUSEX);
				set_speed(mousey);
			} else {
				setting_speed = 0;
				unqdevice(MOUSEX);
				unqdevice(MOUSEY);
			}
		} else 
#endif
		if (cur_gid == srcwin) {
			selecting = do_select(v);
		} else if ((v == 0) && selecting) {
			selecting = do_select(v);
		}
		if (v) {
			leftmouse(mousex, mousey);
		}
		break;
	  case WMTXCLOSE:
		deadwin = pid_to_wintty(v);
		if (deadwin == cur_win) {
			cur_win = dbx_win;
			winset(dbx_win->wt_gid);
		}
		if (deadwin != (WINTTYMAP *) 0) {
			byebye(deadwin);
			free_wintty(deadwin);
		}
		break;
	  case QPTY_CANREAD:
		if (v == syncptyin.sp_ptynum) {
			process_shm();
		} else {
			read_win = ptynum_to_window(v);
			read_win->need_read = 1;
		}
		break;
	  case QPTY_STOP:
		cur_win = ptynum_to_window(v);
		cur_win->stopped = 1;
		cur_win->need_update = 1;
		break;
	  case QPTY_START:
		cur_win = ptynum_to_window(v);
		stopped = 0;
		cur_win->need_update = 1;
		break;
	}
}


/*
 * Send a character to the shell
 */
void
send_shell(wtmp, buf, n)
WINTTYMAP	*wtmp;
char	*buf;
int	n;
{
	int nb;

	if ((nb = write(wtmp->wt_masterfd, buf, n)) < 0)
		byebye();
	if (flag_script)
		(void) write(script_fd, buf, n);
}

/*
 * This procedure manages a shell
 */
void
dbxtool()
{
	register int i;
	short t, v;
	char	rbuff[1000];
	int	nb;

	dbx_win->realcursorcolor = 2;
	if (user_win) {
		user_win->realcursorcolor = 2;
	}
	dbx_win->disconnected = 1;
	if (user_win) {
		user_win->disconnected = 1;
	}
	cur_win = dbx_win;
	read_win = dbx_win;

	/*
	 * We use TIMER0 to enforce a slight delay in updating the screen
	 * when we get small amounts of data (< 200 characters).  We
	 * make the delay smallish so that the users typing will work well,
	 * but large enough to let the shell process send us some data.
	 */
/*
	noise(TIMER0, 8);
*/
	qdevice(MENUBUTTON);
	qdevice(MIDDLEMOUSE);
	qdevice(LEFTMOUSE);
	tie(LEFTMOUSE, MOUSEX, MOUSEY);
	tie(MENUBUTTON, MOUSEX, MOUSEY);
	tie(MIDDLEMOUSE, MOUSEX, MOUSEY);
	for (i = 1; i <= MAXKBDBUT; i++)
		qdevice(i);

	/*
	 * Now run the shell...
	 */
	for (;;) {
		t = qread(&v);
		queue_cmd(t, v);
		if (read_win->need_read || read_win->need_update) {
				update(read_win);
/*
			if ((read_win->read_bytes > 200) || read_win->need_update)
			else
				qdevice(TIMER0);
*/
			for (;;) {
				nb = win_read(read_win, rbuff, sizeof(rbuff));
				win_write(read_win, rbuff, nb);
				if (nb == 0) {
					break;
				}
			}
			update(read_win);
		}
	}
}




dbxredisp() {
	int	curwin;
	int xlen, ylen;
	int xorg, yorg;
	int xlast, ylast;
	rect_t r, r1;
	static int oxlen, oylen;
	int tmp;
	int i;


	winset(dbx_win->wt_gid);
	getsize(&xlen, &ylen);
	r.xlen = (short) xlen;
	r.ylen = (short) ylen;
	r.xorg = 0;
	r.yorg = 0;
	getorigin(&xorg, &yorg);
	dbx_win->position.xorg = (short) xorg;
	dbx_win->position.yorg = (short) yorg;
	dbx_win->position.xlen = (short) xlen;
	dbx_win->position.ylen = (short) ylen;
	setoutput(&r);
	color(dpagecolor);
	rectfi(r.xorg, r.yorg, xlen, ylen);
	redraw_buttons(&(r));
#ifdef TF
	tvviewsize(dbx_win->wt_tv, xlen - 100, ylen);
	tvdraw(dbx_win->wt_tv, 1);
#else
	r.xlen = (short) xlen - 100;
	r.xorg = 3;
	setoutput(&r);
	tf_redraw(dbx_win->wt_textnum, &(r));
#endif
	dbx_win->need_redisplay = 0;
}
