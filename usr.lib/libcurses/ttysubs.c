#include "curses.h"

#define EOFCHAR	4
#define MINCHAR	1

char _saveeof;

raw()
{
	_tty.c_iflag &= ~(INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IUCLC|IXON);
	_tty.c_oflag &= ~OPOST;
	_tty.c_cflag &= ~(CSIZE|PARENB);
	_tty.c_cflag |= CS8;
	_tty.c_lflag &= ~(ISIG|ICANON);
	_tty.c_cc[EOFCHAR] = MINCHAR;
	_pfast=_rawmode=TRUE;
	ioctl(_tty_ch, TCSETA, &_tty);
}

noraw()
{
	_tty.c_iflag |= ISTRIP|IXON;
	if (_res_flg.c_iflag & ICRNL)
		_tty.c_iflag |= ICRNL;
	_tty.c_oflag |= OPOST;
	_tty.c_lflag |= ISIG|ICANON;
	_tty.c_cc[EOFCHAR] = _saveeof;
	_rawmode=FALSE;
	_pfast=!(_res_flg.c_iflag&ICRNL);
	ioctl(_tty_ch, TCSETA, &_tty);
}

crmode()
{
	_tty.c_lflag &= ~ICANON;
	_tty.c_cc[EOFCHAR] = MINCHAR;
	_rawmode = TRUE;
	ioctl(_tty_ch, TCSETA, &_tty);
}

nocrmode()
{
	_tty.c_lflag |= ICANON;
	_tty.c_cc[EOFCHAR] = _saveeof;
	_rawmode=FALSE;
	ioctl(_tty_ch, TCSETA, &_tty);
}

echo()
{
	_tty.c_lflag |= ECHO;
	_echoit = TRUE;
	ioctl(_tty_ch, TCSETA, &_tty);
}

noecho()
{
	_tty.c_lflag &= ~ECHO;
	_echoit = FALSE;
	ioctl(_tty_ch, TCSETA, &_tty);
}

nl()
{
	_tty.c_iflag |= ICRNL;
	_tty.c_oflag |= ONLCR;
	_pfast = _rawmode;
	ioctl(_tty_ch, TCSETA, &_tty);
}

nonl()
{
	_tty.c_iflag &= ~ICRNL;
	_tty.c_oflag &= ~ONLCR;
	_pfast = TRUE;
	ioctl(_tty_ch, TCSETA, &_tty);
}

savetty()
{
	ioctl(_tty_ch, TCGETA, &_tty);
	_res_flg = _tty;
	_saveeof = _tty.c_cc[EOFCHAR];
}

resetty()
{
	_tty = _res_flg;
	ioctl(_tty_ch, TCSETAW, &_tty);
}
