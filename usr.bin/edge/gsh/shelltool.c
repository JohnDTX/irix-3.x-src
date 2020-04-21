/*
 * This procedure runs the shell...
 */
#include <sys/pty_ioctl.h>
#include <termio.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <gl.h>
#include <device.h>
#include "window.h"
#include "gsh.h"
#include "kb.h"

extern	int	master_fd;
extern	int	script_fd;

char	need_read, need_update, need_redisplay;
char	stopped, disconnected, cursor_change;
int	read_bytes;
int	cursor_on;

rect_t	tview_r;

/*
 * When we need to redisplay ourselves, this is what we do
 */
void
redisplay()
{
	int xlen, ylen;
	int xlast, ylast;
	rect_t r;
	static struct winsize ws;
	int rows, cols;
	extern int last_rows, last_cols;

#ifdef	SHRINK
	if (!opened) {
		img_drawit();
		return;
	}
#endif

	getsize(&xlen, &ylen);
	xlast = xlen - 1;
	ylast = ylen - 1;

	/*
	 * Draw borders.  Draw a box around everything, then draw a
	 * line between the scroll bar and the textframe.  Then put
	 * up a one pixel border around the textframe.
	 */
	r.xorg = 0;
	r.yorg = 0;
	r.xlen = xlen;
	r.ylen = ylen;
	setoutput(&r);
	color(BLACK);
	rects(0, 0, xlast, ylast);
	move2s(FRAMEXWIDTH, FRAMEYWIDTH);
	draw2s(FRAMEXWIDTH, ylast);
	color(txport[0].tx_pagecolor);
	rects(FRAMEXWIDTH+LINXWIDTH, FRAMEYWIDTH,
				     xlast - FRAMEXWIDTH,
				     ylast - FRAMEYWIDTH);

	/*
	 * Now update the text frame
	 */
	r.xorg = FRAMEXWIDTH + LINXWIDTH + TFXWIDTH;
	r.yorg = FRAMEYWIDTH + TFYWIDTH;
	r.xlen = xlen - r.xorg;
	r.ylen = ylen - r.yorg;
	rows = txport[0].tx_rows;
	cols = txport[0].tx_cols;
	tf_redraw(0, &r);

	/* send process a SIGWINCH if the window size changed */
	if ((ws.ws_row != txport[0].tx_rows) ||
	    (ws.ws_col != txport[0].tx_cols)) {
		ws.ws_row = txport[0].tx_rows;
		ws.ws_col = txport[0].tx_cols;
		ws.ws_xpixel = r.xlen;
		ws.ws_ypixel = r.ylen;
		if (ioctl(master_fd, TIOCSWINSZ, &ws) >= 0) {
			/*
			 * Only send the signal if the kernel can support
			 * the ioctl.  This avoids bad behaviour on old
			 * kernels.
			 */
			kill(-child_pid, SIGWINCH);
		}
		last_rows = rows;
		last_cols = cols;
	}

	need_redisplay = 0;
}

/*
 * Update the text display, modifying the cursor color according to
 * external state
 */
void
update()
{
	register struct txport *tx;

	tx = &txport[0];
	if (stopped)
		tx->tx_state &= ~TX_GOING;
	else
		tx->tx_state |= TX_GOING;

	if (disconnected)
		tx->tx_state &= ~TX_SELECTED;
	else
		tx->tx_state |= TX_SELECTED;

	if (cursor_change && opened) {
		/*
		 * Some kind of cursor changing event occured.
		 * Update cursor color and blink colors.
		 */
		if (tx->tx_state & TX_BLINKING)
			blink(0, tx->tx_cursorcolor, 0, 0, 0);	/* stop ! */
		if (tx->tx_state & TX_GOING) {
			/*
			 * Restore cursor to original color mapping
			 */
			mapcolor(tx->tx_cursorcolor,
				 tx->tx_cursor_r, tx->tx_cursor_g,
				 tx->tx_cursor_b);
		} else {
			/*
			 * When window is stopped, set cursor to red
			 * (like a stop sign)
			 */
			mapcolor(tx->tx_cursorcolor, 255, 0, 0);
		}
		tx_setblink(tx);			/* start going */
	}

	cursor_change = 0;
	need_update = 0;
	if (!stopped && opened)
		tx_update();
}

/*
 * Send a keyboard character to the shell.  What we recieve here is a
 * raw keyboard button press.  Translate the raw button into a given
 * ascii key code according to a lookup function.
 */
void
write_shell(stroke, key)
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
					ioctl(master_fd, TCSBRK, 0);
			} else {
				c = *k;
				if (write(master_fd, &c, 1) < 0)
					byebye();
			}
		}
	}
}

/*
 * Handle a command from the graphics queue
 */
void
queue_cmd(t, v)
	short t, v;
{
	/* check for keyboard buttons */
	if (t <= MAXKBDBUT) {
		write_shell(v, t);
		return;
	}

	/* handle the rest of the events */
	switch (t) {
	  case TIMER0:
		unqdevice(TIMER0);
		if (read_bytes || need_update) {
			read_bytes = 0;
			update();
		}
		break;
	  case REDRAW:
		need_redisplay = 1;
		break;
	  case INPUTCHANGE:
		cursor_change = 1;
		if (v == 0) {
			disconnected = 1;
			need_update = 1;
		} else {
			disconnected = 0;
			need_update = 1;
			kb_fix();
		}
		break;
	  case MENUBUTTON:
		if (v)
			domenu();
		break;
	  case WMTXCLOSE:
		if ((v == 777) && !flag_hold)
			byebye();
		break;
	  case QPTY_CANREAD:
		need_read = 1;
		break;
	  case QPTY_STOP:
		cursor_change = 1;
		stopped = 1;
		need_update = 1;
		break;
	  case QPTY_START:
		cursor_change = 1;
		stopped = 0;
		need_update = 1;
		break;
	}
}

/*
 * Read from the shell, writing to our textport.  Make sure we drain
 * everything waiting from the shell...We do partial updates if we
 * are getting a large amount of data, mostly to make it look nice.
 * This consumes more cpu cycles, but it really improves the visual
 * behaviour.
 */
void
read_shell()
{
	char buf[100];
	int nb;
	short t, v;

	need_read = 0;			/* we are doing it now! */
	for (;;) {
		nb = read(master_fd, buf, sizeof(buf));
		(void) ioctl(master_fd, PTIOC_QUEUE, 0);
		if (nb < 0) {
			if (errno == EAGAIN)		/* streams!#^#$%&^$ */
				break;
			byebye();
		}
		if (nb == 0)
			break;
		if (flag_script)
			(void) write(script_fd, buf, (unsigned) nb);
		read_bytes += nb;
		tx_addchars(0, buf, nb);
		if ((read_bytes > 300) || need_update) {
			update();
			read_bytes = 0;
		}

		/*
		 * Now poll graphics queue, looking for a possible keyboard
		 * command (like ^S/^Q/interrupt, etc)
		 */
		while (softqtest()) {
			t = softqread(&v);
			queue_cmd(t, v);
			if (need_redisplay)
				redisplay();
		}
	}
}

/*
 * Send a character to the shell
 * XXX fix to deal with incomplete writes...
 */
void
send_shell(buf, n)
	char *buf;
	int n;
{
	if (write(master_fd, buf, (unsigned) n) < 0)
		byebye();
	if (flag_script)
		(void) write(script_fd, buf, (unsigned) n);
}

/*
 * This procedure manages a shell
 */
void
shelltool()
{
	register int i;
	short t, v;

	disconnected = 1;

	/*
	 * We use TIMER0 to enforce a slight delay in updating the screen
	 * when we get small amounts of data (< 200 characters).  We
	 * make the delay smallish so that the users typing will work well,
	 * but large enough to let the shell process send us some data.
	 */
	noise(TIMER0, 3);		/* ~ 20 times a second (66/3) */
	qdevice(MENUBUTTON);
	for (i = 1; i <= MAXKBDBUT; i++)
		qdevice(i);

	/*
	 * Now run the shell...
	 */
	redisplay();
	last_rows = txport[0].tx_rows;
	last_cols = txport[0].tx_cols;
	for (;;) {
		t = softqread(&v);
		queue_cmd(t, v);
		if (need_redisplay)
			redisplay();
		if (need_read || need_update) {
			read_shell();
			if ((read_bytes > 300) || need_update)
				update();
			else
				qdevice(TIMER0);
		}
	}
}
