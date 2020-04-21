static	char	*Io_c	= "@(#)io.c	1.19 6/14/85";
/*
 * Various input/output functions
 *
 * @(#)io.c	2.15 (Berkeley) 2/11/81
 * @(#)io.c	1.19 6/14/85 16:50:22
 */
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include "rogue.h"

extern	int	errno;
extern	int	myerrno;
	int	newmin;		/* status() wants Readchar() to alarm(newmin) */

WINDOW	*msgw;
WINDOW	*oldmsgw;

extern	struct	tm	*localtime();

char *wordsplit();

/*
 * wmsg:
 *	Display a message at the top of the screen using specified window.
 */

wmsg(win,fmt,a1,a2,a3,a4,a5)
WINDOW	*win;
char	*fmt, *a1, *a2, *a3, *a4, *a5;
{

    oldmsgw = msgw;
    msgw = win;
    msg(fmt,a1,a2,a3,a4,a5);
    msgw = oldmsgw;
}

/*
 * msg:
 *	Display a message at the top of the screen.
 */

msg(fmt,a1,a2,a3,a4,a5)
char	*fmt, *a1, *a2, *a3, *a4, *a5;
{
    reg1 rega1	char	*cp;
		char	buf[BUFSIZ];
		int	i;
		char	*p;
#ifdef	COLOR
		PIXEL	oldcolor;
#endif
		int	msglen;
#ifdef	iris
		Icoord	x;
    static	char	pstr[2];
		static	PIXEL	lastcolor = -1;
#endif

#ifdef	iris
#ifndef	gl2
    if (lastcolor == -1 && _iris)
	lastcolor = msgw->_attrib;
#endif	gl2
#endif	iris
    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0') {
  	wmove(msgw, 0, 0);
#ifdef	COLOR
	acolor(msgw, Dforeground, Dbackground);
#endif
	wclrtoeol(msgw);
	mpos = 0;
	return;
    }
#ifdef	COLOR
    Rcolor = msgw->_attrib;
#endif
    /*
     * Do the printf into buf
     */
    sprintf(buf,fmt,a1,a2,a3,a4,a5);
    /*
     * Display it (giving him a chance to see the previous one if it is up
     * there with the --More--)
     */
    strcpy(huh, buf);
    if (mpos) {
#ifdef	COLOR
	oldcolor = msgw->_attrib;
	acolor(msgw, Dforeground, Dbackground);
#endif
#ifdef	iris
#ifndef	gl2
	if (!_iris)
#endif	gl2
#endif	iris
	{
		wmove(msgw, 0, mpos);
		waddstr(msgw, more);
	}
	Draw(msgw);
#ifdef	iris
#ifndef	gl2
	if (_iris) {
	    cmov2i((Icoord)(mpos),_linblit[0+1]);
	    if (_nowfont != regfont);
		font(_nowfont=regfont);
	    if (dbcolor(lastcolor) == Dforeground)
		color(Dbackground);
	    else
		color(Dforeground);
	    charstr(more);
	    pstr[0] = cursor;
	    if (dbcolor(lastcolor) == C_red)
		color(C_blue);
	    else
		color(C_red);
	    charstr(pstr);
	    color(Dforeground);
	}
#endif	gl2
#endif	iris
	wait_for(' ');
#ifdef	COLOR
	msgw->_attrib = oldcolor;
#endif
    }
#ifdef	iris
#ifndef	gl2
    if (_iris) {
	lastcolor = msgw->_attrib;
	if (_nowfont != msgfont);
	    font(_nowfont=msgfont);
	msglen = strwidth(buf) + 1;
	if (msglen < 1)
	    msglen = XMAXSCREEN;
    } else
#endif	gl2
#endif	iris
	msglen = strlen(buf);
	if (msglen > maxmsg) {
	i = 1;
	while (wordlen(buf,i) <= maxmsg)
		i++;
	i--;
	if (i <= 0)
	    i = 1;
	p = wordsplit(buf,i);
	*p = '\0';
    }
    mvwaddstr(msgw, 0, 0, buf);
#ifdef	COLOR
    oldcolor = msgw->_attrib;
    acolor(msgw, Dforeground, Dbackground);
#endif
    wclrtoeol(msgw);
    mpos = msglen;
    Draw(msgw);
    if (msglen > maxmsg) {
#ifdef	iris
#ifndef	gl2
	if (_iris) {
	    if (_nowfont != msgfont);
		font(_nowfont=msgfont);
	    mpos = strwidth(buf) + 1;
	} else
#endif	gl2
#endif	iris
	    mpos = strlen(buf);
#ifdef	COLOR
	msgw->_attrib = oldcolor;
#endif
	msg(p+1);
    }
}

char *
wordsplit(s,i)
reg1 rega1	char	*s;
		int	i;
{
    for (;;)
	if (*s++ == ' ' && --i <= 0)
	    return --s;
}

wordlen(s,i)
reg1 rega1	char	*s;
		int	i;
{
    char	*p;

    p = wordsplit(s,i);
    *p = '\0';
#ifdef	iris
#ifndef	gl2
    if (_iris)
	i = strwidth(s) + 1;
    else
#endif	gl2
#endif	iris
	i = strlen(s);
    *p = ' ';
    return i;
}

#ifndef FAST
/*
 * winat:
 *	What is there in the dungeon, looks on monster and real window
 */

winat(y, x)
reg1 regd1	int	y;
reg2 regd2	int	x;
{
    reg3 regd3	char	mch;
    reg4 regd4	char	ch;

    ch = mvwinch(stdscr, y, x);
    mch = mvwinch(mw, y, x);
    if (mch == ' ')
	return(ch);
    else
	if (!step_ok(ch)) {
	    sprintf(prbuf, "Monster(%s) on top of something bad(%s)",
	      unctrl(mch), unctrl(ch));
	    debug(prbuf);
	    return(NULL);
	} else
	    return(mch);
}
#endif

/*
 * step_ok:
 *	returns true if it is ok to step on ch
 */

step_ok(ch)
{
    switch (ch) {
	case GOLD:		/* may need to check for other steppables */
	case POTION:
	case SCROLL:
	case WAND:
	case RING:
	case STAIRS:
	case FOOD:
	case TRAP:
	case WEAPON:
	case ARMOR:
	case AMULET:
	    return TRUE;
	default:
	    if (ch == door || ch == floor || ch == passage)
		return TRUE;
	    return FALSE;
    }
}

/*
 * Readchar:
 *	calls readchar(), allowing alarm clock interrupts to update time on
 *	status line. Should be used only by command() with ordinary window
 *	being displayed.
 */
Readchar()
{
    int i;

    fflush(stdout);
    alarm(newmin);
    i = readchar();
    alarm(0);
    return i;
}

/*
 * readchar:
 *	flushes stdout so that screen is up to date and then returns
 *	getchar.
 */

readchar()
{
    char	c;
    int		i;
#ifdef	STAND
    extern int	errno;
#endif	STAND

    fflush(stdout);
#ifdef	iris
    if (_iris) {
        while (myerrno || (i = _Getchar()) < 0) {
#ifndef	STAND
            if (myerrno == EINTR) {
                status();
		alarm(newmin);
		myerrno = errno = 0;
	    }
#endif	STAND
        }
        c = i;
    } else
#endif	iris
#ifdef	STAND
    while (myerrno || (c=getchar()) < 0) {
	if (myerrno == EINTR) {
	    status();
	    alarm(newmin);
	    myerrno = 0;
	}
    }
#else	STAND
    while (myerrno || read(0, &c, 1) < 0) {
        if (myerrno == EINTR) {
	    status();
	    alarm(newmin);
            myerrno = 0;
	}
    }
#endif	STAND
    return c;
}

/*
 * unctrl:
 *	Print a readable version of a certain character
 */

char	*
unctrl(ch)
char	ch;
{
    extern char	*_unctrl[];		/* Defined in curses library */

    return _unctrl[ch&0177];
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

status()
{
    reg1 regd1	int	oy;
    reg2 regd2	int	ox;
    reg3 rega1	char	*pb;
    static	char	buf[80];
    static	int	hpwidth = 0;
    static	int	s_lev = -1, s_pur = -1, s_hp = -1, s_str = -1;
    static	int	s_add = -1, s_ac = -32768, s_lvl = -1;
    static	long s_exp = -1;
    static	int	s_min = -1, s_hour = -1;
    static	int	s_hung = H_NONE;
    static	int	Refresh = 2;	/* non-zero forces re-draw */
    static	time_t LRefresh = 0;	/* more than 5 seconds undoes reverse video */
		char	*hptr;		/* hungry ptr */
		time_t	now;		/* current time */
	struct	tm	*tm;
#ifdef	COLOR
		int	hcolor;		/* hungry color */
#endif
		time_t	time();

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    now = time((time_t *)0);
    tm = localtime(&now);
    newmin = 60-tm->tm_sec+1;	/* update clock if no typing & new time */
    if (s_hp == pstats.s_hpt && s_lvl == pstats.s_lvl && s_exp == pstats.s_exp
      && s_pur == purse
      && s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm)
      && s_str == pstats.s_str.st_str && s_add == pstats.s_str.st_add
      && s_min == tm->tm_min && s_hour == tm->tm_hour
      && s_lev == level && s_hung == hunglev) {
	    if (!Refresh)
	        return;
    } else
	Refresh = 2;	/* refresh next time around to clear reverse video */

    getyx(cw, oy, ox);
    wmove(cw, LINES - 1, 0);
    wclrtoeol(cw);		/* so Curses knows color change is change UGH */
    wmove(cw, LINES - 1, 0);
#ifdef	COLOR
    if (level == s_lev)
	acolor(cw, C_green, Dbackground);
    else
	acolor(cw, Dbackground, C_green);
#endif
    wprintw(cw, "Level: %d", level);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    wprintw(cw, "  ");
#ifdef	COLOR
    if (s_pur == purse)
	acolor(cw, C_gold, Dbackground);
    else
	acolor(cw, Dbackground, C_gold);
#endif
    wprintw(cw, "Gold: %-3d", purse);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    wprintw(cw, "  ");
#ifdef	COLOR
    if (s_hp == pstats.s_hpt)
	acolor(cw, C_red, Dbackground);
    else
	acolor(cw, Dbackground, C_red);
#endif
    wprintw(cw, "Hp: %*d(%*d)", hpwidth ,pstats.s_hpt, hpwidth, max_hp);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    wprintw(cw, "  ");
#ifdef	COLOR
    if (s_str == pstats.s_str.st_str && s_add == pstats.s_str.st_add)
	acolor(cw, C_blue, Dbackground);
    else
	acolor(cw, Dbackground, C_blue);
#endif
    wprintw(cw, "Str: %-2d", pstats.s_str.st_str);
    if (pstats.s_str.st_add)
	wprintw(cw, "/%d", pstats.s_str.st_add);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    wprintw(cw, "  ");
#ifdef	COLOR
    if (s_ac == (cur_armor!=NULL ? cur_armor->o_ac:pstats.s_arm))
	acolor(cw, C_green, Dbackground);
    else
	acolor(cw, Dbackground, C_green);
#endif
    wprintw(cw, "Arm: %-2d",
      cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    wprintw(cw, "  ");
#ifdef	COLOR
    if (s_lvl == pstats.s_lvl && s_exp == pstats.s_exp)
	acolor(cw, C_magenta, Dbackground);
    else
	acolor(cw, Dbackground, C_magenta);
#endif
    wprintw(cw, "Exp: %d/%ld ", pstats.s_lvl, pstats.s_exp);
    switch (hunglev) {
      case H_NONE:
	hptr = "";
#ifdef	COLOR
	hcolor = C_green;
#endif
	break;
      case H_SOME:
	hptr = "Hungry";
#ifdef	COLOR
	hcolor = C_blue;
#endif
	break;
      case H_VERY:
	hptr = "Weak";
#ifdef	COLOR
	hcolor = C_yellow;
#endif
	break;
      case H_FAINT:
	hptr = "Fainting";
#ifdef	COLOR
	hcolor = C_red;
#endif
	break;
    }
#ifdef	COLOR
    if (hunglev == s_hung)
	acolor(cw, hcolor, Dbackground);
    else
	acolor(cw, Dbackground, hcolor);
#endif
    wprintw(cw, "%s", hptr);
    wprintw(cw, "  ");
#ifdef	COLOR
    if (s_hour != tm->tm_hour) {
	acolor(cw, Dbackground, C_red);
	newmin = 7;		/* turn off reverse video in 6 or 7 seconds */
    } else if (s_min != tm->tm_min && !(tm->tm_min % 5)) {
	acolor(cw, Dbackground, C_blue);
	newmin = 7;		/* turn off reverse video in 6 or 7 seconds */
    } else
	acolor(cw, C_blue, Dbackground);
#endif
    wprintw(cw, "%02d%02d", tm->tm_hour, tm->tm_min);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
/*  wclrtoeol(cw); */		/* not necessary if above one left in */
    wmove(cw, oy, ox);
    /*
     * Save old status
     */
    if (now - LRefresh > 5 || now - LRefresh < 0) {
	s_lev = level;
	s_pur = purse;
	s_hp = pstats.s_hpt;
	s_str = pstats.s_str.st_str;
	s_add = pstats.s_str.st_add;
	s_lvl = pstats.s_lvl; 
	s_exp = pstats.s_exp; 
	s_ac = (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
	s_hung = hunglev;
	s_min = tm->tm_min;
	s_hour = tm->tm_hour;
	LRefresh = time((time_t)0);
	Refresh--;
    }
    Draw(cw);
}

/*
 * wait_prompt
 *	Display a prompt and wait till she types the right key
 */

wait_prompt(win, ch)
WINDOW	*win;
char	ch;
{
	prompt(win);
	wait_for(ch);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */

wait_for(ch)
reg1 regd1	char	ch;
{
    reg2 regd2	char	c;

    if (ch == '\n')
        while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    else
        while (readchar() != ch)
	    continue;
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */

show_win(scr, message)
reg1 rega1	WINDOW	*scr;
		char	*message;
{
    if (message && *message) {
	mvwaddstr(scr, 0, 0, message);
	touchwin(scr);
	wmove(scr, hero.y, hero.x);
    } else
	touchwin(scr);
    Draw(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
 * Dcolor
 *	switches the color in preperation for displaying character ch
 *	at location (y,x) on window win.
 */

#ifdef	COLOR
Dcolor(win,ch)
WINDOW	*win;
uchar	ch;
{
    int		fore;
    int		back;

    fore = Dforeground;
    back = Dbackground;
    switch (ch) {
      when FOOD:
	fore = C_green;
      when WEAPON:
	fore = C_magenta;
      when ARMOR:
	fore = C_cyan;
      when GOLD:
	fore = C_gold;
      when STAIRS:
	fore = C_red;
      when TRAP:
	fore = C_red;
      when PLAYER:
	fore = C_PLAYER;
      when SCROLL:
	fore = C_magenta;
      when POTION:
	fore = C_green;
      when WAND:
	fore = C_magenta;
      when RING:
	fore = C_magenta;
      when MAGIC:
	fore = C_magenta;
      when AMULET:
	fore = C_gold;
	back = C_blue;
      when ZAP:
	fore = C_red;
      otherwise:
	if (isupper(ch)) {
	    fore = C_red;
	} else if (ch == wallv || ch == wallh || ch == wallll ||
	  ch == walllr || ch == wallul || ch == wallur || ch == passage)
	    fore = C_cyan;
	else if (ch == floor)
	    fore = C_blue;
	else if (ch == door)
	    fore = C_cyan;
    }
    if (fore != Dforeground || back != Dbackground)
	acolor(win, fore, back);
}
#endif

#ifdef	STAND
atoi(s)
char	*s;
{
    int		n;

    n = 0;
    while (*s)
	n = n*10 + (*s++ -'0');
    return n;
}

exit(n)
{
	printf("exit(%d) looping:",n);
loop:
	goto loop;
}
#endif

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char	*
vowelstr(str)
char	*str;
{
    switch (*str) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	    return "n";
	default:
	    return "";
    }
}
