static	char	*Ttyctl_c	= "@(#)ttyctl.c	1.5";
/*
 *	Functions to change the ioctrl of a tty.
 *	replace the old defines (for stty calls) from curses.h 
 */

/*
 *	Old mode "cbreak", set in the "crmode" function (!),
 *	is now approximately equivalent to "-icanon";
 *	old mode "raw" is now a conglomerate of many things;
 *	both require the setting of new VMIN and VTIME values to
 *	cause the characters to be passed directly to the requesting
 *	program.
 *	For some odd reason, what the curses lib. called the "nl" function
 *	actually set the CRMOD bit of the old sgtty structure - which is
 *	really an "stty -nl" command; hense the arg value of "-nl" in the
 *	"nl" function...
 */

#include "curses.ext"

#ifdef	V7

raw()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags |= RAW;
	_pfast=_rawmode=TRUE;
	stty(_tty_ch,&_tty);
}

noraw()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags &= ~RAW;
	_rawmode=FALSE;
	_pfast=!(_tty.sg_flags &CRMOD);
	stty(_tty_ch,&_tty);
}

crmode()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags |= CBREAK;
	_rawmode=TRUE;
	stty(_tty_ch,&_tty);
}

nocrmode()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags &= ~CBREAK;
	_rawmode=FALSE;
	stty(_tty_ch,&_tty);
}

echo()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags |= ECHO;
	_echoit = TRUE;
	stty(_tty_ch, &_tty);
}

noecho()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags &= ~ECHO;
	_echoit=FALSE;
	stty(_tty_ch, &_tty);
}

nl()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags |= CRMOD;
	_pfast = _rawmode;
	stty(_tty_ch, &_tty);
}

nonl()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags &= ~CRMOD;
	_pfast = TRUE;
	stty(_tty_ch, &_tty);
}

savetty()
{
	gtty(_tty_ch, &_tty);
	_res_flg = _tty.sg_flags;
}

resetty()
{
	gtty(_tty_ch,&_tty);
	_tty.sg_flags = _res_flg;
	stty(_tty_ch, &_tty);
}

flushi()
{
	gtty(_tty_ch,&_tty);
	stty(_tty_ch, &_tty);
}


#else

struct mds {
	char	*string;
	int	set;
	int	reset;
};

struct mds cmodes[] = {
/*
 *	"raw", CS8, (CSIZE|PARENB),
 *	"-raw", (CS7|PARENB), CSIZE,
 */
	"raw", CS8, CSIZE,
	"-raw", CS7, CSIZE,
	0
};

struct mds imodes[] = {
	"-nl", ICRNL, (INLCR|IGNCR),
	"nl", 0, ICRNL,
	"raw", 0, -1,
	"-raw", (BRKINT|IGNPAR|ISTRIP|ICRNL|IXON), 0,
	0
};

struct mds lmodes[] = {
	"icanon", ICANON, 0,
	"-icanon", 0, ICANON,
	"echo", ECHO, 0,
	"-echo", 0, ECHO,
	"raw", 0, (ISIG|ICANON|XCASE),
	"-raw", (ISIG|ICANON), 0,
	0
};

struct mds omodes[] = {
	"-nl", ONLCR, (OCRNL|ONLRET),
	"nl", 0, ONLCR,
	"raw", 0, OPOST,
	"-raw", OPOST, 0,
	0
};

char	*arg;
unsigned c_ccsav[NCC];

raw()
{
	int	i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "raw";
	_pfast = _rawmode = TRUE;
	c_ccsav[VEOF] = _tty.c_cc[VEOF];
	c_ccsav[VEOL] = _tty.c_cc[VEOL];
	_tty.c_cc[VMIN] = 1;
	_tty.c_cc[VTIME] = 1;
	for(i=0; imodes[i].string; i++)
		if(eq(imodes[i].string)) {
			_tty.c_iflag &= ~imodes[i].reset;
			_tty.c_iflag |= imodes[i].set;
		}
	for(i=0; omodes[i].string; i++)
		if(eq(omodes[i].string)) {
			_tty.c_oflag &= ~omodes[i].reset;
			_tty.c_oflag |= omodes[i].set;
		}
	for(i=0; cmodes[i].string; i++)
		if(eq(cmodes[i].string)) {
			_tty.c_cflag &= ~cmodes[i].reset;
			_tty.c_cflag |= cmodes[i].set;
		}
	for(i=0; lmodes[i].string; i++)
		if(eq(lmodes[i].string)) {
			_tty.c_lflag &= ~lmodes[i].reset;
			_tty.c_lflag |= lmodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

noraw()
{
	int	i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "-raw";
	_rawmode = FALSE;
	_pfast = (_tty.c_lflag & ICANON);
	_tty.c_cc[VEOF] = c_ccsav[VEOF];
	_tty.c_cc[VEOL] = c_ccsav[VEOL];
	for(i=0; imodes[i].string; i++)
		if(eq(imodes[i].string)) {
			_tty.c_iflag &= ~imodes[i].reset;
			_tty.c_iflag |= imodes[i].set;
		}
	for(i=0; omodes[i].string; i++)
		if(eq(omodes[i].string)) {
			_tty.c_oflag &= ~omodes[i].reset;
			_tty.c_oflag |= omodes[i].set;
		}
	for(i=0; cmodes[i].string; i++)
		if(eq(cmodes[i].string)) {
			_tty.c_cflag &= ~cmodes[i].reset;
			_tty.c_cflag |= cmodes[i].set;
		}
	for(i=0; lmodes[i].string; i++)
		if(eq(lmodes[i].string)) {
			_tty.c_lflag &= ~lmodes[i].reset;
			_tty.c_lflag |= lmodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

crmode()
{
	int	i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "-icanon";
	_rawmode = TRUE;
	_tty.c_cc[VMIN] = 1;
	_tty.c_cc[VTIME] = 1;
	for(i=0; lmodes[i].string; i++)
		if(eq(lmodes[i].string)) {
			_tty.c_lflag &= ~lmodes[i].reset;
			_tty.c_lflag |= lmodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

nocrmode()
{
	int	i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "icanon";
	_rawmode = FALSE;
	_tty.c_cc[VEOF] = CEOF;
	_tty.c_cc[VEOL] = CNUL;
	for(i=0; lmodes[i].string; i++)
		if(eq(lmodes[i].string)) {
			_tty.c_lflag &= ~lmodes[i].reset;
			_tty.c_lflag |= lmodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

echo()
{
	int	i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "echo";
	_echoit = TRUE;
	for(i=0; lmodes[i].string; i++)
		if(eq(lmodes[i].string)) {
			_tty.c_lflag &= ~lmodes[i].reset;
			_tty.c_lflag |= lmodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

noecho()
{
	int i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "-echo";
	_echoit = FALSE;
	for(i=0; lmodes[i].string; i++)
		if(eq(lmodes[i].string)) {
			_tty.c_lflag &= ~lmodes[i].reset;
			_tty.c_lflag |= lmodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

nl()
{
	int i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "-nl";
	_pfast = _rawmode;
	for(i=0; imodes[i].string; i++)
		if(eq(imodes[i].string)) {
			_tty.c_iflag &= ~imodes[i].reset;
			_tty.c_iflag |= imodes[i].set;
		}
	for(i=0; omodes[i].string; i++)
		if(eq(omodes[i].string)) {
			_tty.c_oflag &= ~omodes[i].reset;
			_tty.c_oflag |= omodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

nonl()
{
	int i;

	ioctl(_tty_ch, TCGETA, &_tty);
	arg = "nl";
	_pfast = TRUE;
	for(i=0; imodes[i].string; i++)
		if(eq(imodes[i].string)) {
			_tty.c_iflag &= ~imodes[i].reset;
			_tty.c_iflag |= imodes[i].set;
		}
	for(i=0; omodes[i].string; i++)
		if(eq(omodes[i].string)) {
			_tty.c_oflag &= ~omodes[i].reset;
			_tty.c_oflag |= omodes[i].set;
		}
	ioctl(_tty_ch, TCSETAF, &_tty);
}

eq(string)
char	*string;
{
	register int i;
	register int temp;

	if (!arg)
		return(0);
	i = 0;
loop:
	if (arg[i] != string[i])
		return(0);
	temp = arg[i];
	if (temp == '\0') goto eqdone;
	i++;
	goto loop;
eqdone:
	return(1);
}

savetty()
{
	ioctl(_tty_ch, TCGETA, &_tty_res);
}

resetty()
{
	ioctl(_tty_ch, TCSETAF, &_tty_res);
}

/* flush the input queue */
flushi()
{
	ioctl(_tty_ch, TCFLSH, 0);
}

/* flush the output queue */
flusho()
{
	ioctl(_tty_ch, TCFLSH, 1);
}
#endif	V7
