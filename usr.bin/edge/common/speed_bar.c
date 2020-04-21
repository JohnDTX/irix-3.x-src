/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/speed_bar.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:06 $
 */
#ifdef SPEED_BAR
#include <gl.h>
#include "window.h"
#include "tf.h"
#include "buttons.h"
#include "manage.h"

int	speed_win = -100;
int	current_ht = 150;
#endif
int	current_delay = 6;
#ifdef SPEED_BAR

speed_bar_init()
{
	rect_t	r;
	int	xlen, ylen, xorg, yorg;


	prefsize(30,300);
	speed_win = winopen();
	getsize(&xlen, &ylen);
	getorigin(&xorg, &yorg);

	r.xorg = 0;
	r.yorg = 0;
	r.xlen = xlen;
	r.ylen = ylen;
	setoutput(&r);
	color(WHITE);
	clear();
}

speedredisp()
{
	rect_t	r;
	int	xlen, ylen, xorg, yorg;
	int	cur_win;

	cur_win = winget();
	winset(speed_win);
	getsize(&xlen, &ylen);
	getorigin(&xorg, &yorg);

	r.xorg = 0;
	r.yorg = 0;
	r.xlen = xlen;
	r.ylen = ylen;
	setoutput(&r);
	color(WHITE);
	clear();
	color(CYAN);
	rectfi(0, current_ht-10, xlen, current_ht);
	winset(cur_win);
}

set_speed(y)
int	y;
{
	int	xorg, yorg;
	int	cur_win;

	cur_win = winget();
	winset(speed_win);
	getorigin(&xorg, &yorg);
	current_ht = y - yorg;
	current_delay = (300 - current_ht)/10;
	speedredisp();
	winset(cur_win);
}
#endif

