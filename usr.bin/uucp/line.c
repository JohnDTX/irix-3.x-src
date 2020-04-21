/* @(#)line.c	1.4 */
#include "uucp.h"
#include <sys/types.h>

#define	read	Read
#define	write	Write

#ifndef RT
#include <termio.h>
struct sg_spds {
	int	sp_val;
	int	sp_name;
} spds[] = {
#ifdef	B0
	{   0,    B0},
#endif
#ifdef	B110
	{ 110,  B110},
#endif
#ifdef	B134
	{ 134,  B134},
#endif
#ifdef	B150
	{ 150,  B150},
#endif
#ifdef	B300
	{ 300,  B300},
#endif
#ifdef	B1200
	{1200, B1200},
#endif
#ifdef	B2400
	{2400, B2400},
#endif
#ifdef	B4800
	{4800, B4800},
#endif
#ifdef	B9600
	{9600, B9600},
#endif
#ifdef	B19200
	{19200,B19200},
#endif
#ifdef	EXTA
	{19200, EXTA},
#endif
	{-1, -1}
};
static struct termio Savettyb;
/*
 * set speed/echo/mode...
 *	tty 	-> terminal name
 *	spwant 	-> speed
 * return:  
 *	none
 */
fixline(tty, spwant)
int tty, spwant;
{
	register struct sg_spds *ps;
	struct termio ttbuf;
	int speed = -1;
	int ret;

 	DEBUG(8, "fixline:tty=%d\n", tty);
 	DEBUG(8, "fixline:spwant=%d\n", spwant);
	if (!isatty(tty)) {
		DEBUG(8, "fixline:not a tty so just returning\n", NULL);
		return(0);
	}
#ifdef	XNS
	DEBUG(8, "fixline:XNSSPEED is %d\n", XNSSPEED);
	if (spwant != XNSSPEED)
#endif XNS
	{
		DEBUG(8, "fixline:serial:searching speed table\n", NULL);
		for (ps = spds; ps->sp_val >= 0; ps++) {
			DEBUG(8, "fixline:trying speed %d\n", ps->sp_val);
			if (ps->sp_val == spwant) {
				DEBUG(8, "fixline:found speed, code:%d\n", ps->sp_name);
				speed = ps->sp_name;
			}
		}
		DEBUG(4, "fixline - speed= %d\n", speed);
		ASSERT(speed >= 0, "BAD SPEED", "", speed);
	}
	ioctl(tty, TCGETA, &ttbuf);
	ttbuf.c_iflag = (ushort)0;
	ttbuf.c_oflag = (ushort)0;
	ttbuf.c_lflag = (ushort)0;
#ifdef	XNS
	if (spwant == XNSSPEED)
		ttbuf.c_cflag = ((ttbuf.c_cflag&CBAUD)|CS8|HUPCL|CREAD);
	else
#endif XNS
		if (speed)
			ttbuf.c_cflag = speed;
/* 			ttbuf.c_cflag = (speed|CS8|HUPCL|CREAD); 4.2 */
		else
			ttbuf.c_cflag &= CBAUD;
	ttbuf.c_cflag |= (CS8|CREAD| (speed ? HUPCL : 0));
	ttbuf.c_cc[VMIN] = 6;
	ttbuf.c_cc[VTIME] = 1;
	ret = ioctl(tty, TCSETA, &ttbuf);
	ASSERT(ret >= 0, "RETURN FROM fixline", "", ret);
	return;
}

sethup(dcf)
int dcf;
{
	struct termio ttbuf;

	if(!isatty(dcf)) return(0);
	ioctl(dcf, TCGETA, &ttbuf);
	if(!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		ioctl(dcf, TCSETA, &ttbuf);
	}
}

genbrk(fn)
register int fn;
{
	if(isatty(fn)) ioctl(fn, TCSBRK, 0);
}

/*
 * optimize line setting for sending or receiving files
 * return:
 *	none
 */
#define PACKSIZE	64
#define SNDFILE	'S'
#define RCVFILE 'R'
#define RESET	'X'

setline(type)
/* register */ char type;
{
	static struct termio tbuf, sbuf;
	static int set = 0;

	if(!isatty(Ifn)) return(0);
	DEBUG(2, "setline - %c\n", type);
	switch(type) {
	case SNDFILE:
		break;
	case RCVFILE:
		ioctl(Ifn, TCGETA, &tbuf);
		sbuf = tbuf;
		tbuf.c_cc[VMIN] = PACKSIZE;
		ioctl(Ifn, TCSETAW, &tbuf);
		set++;
		break;
	case RESET:
		if (set == 0) break;
		set = 0;
		ioctl(Ifn, TCSETAW, &sbuf);
		break;
	}
}

savline()
{
	int ret;

	ret = ioctl(0, TCGETA, &Savettyb);
	Savettyb.c_cflag = (Savettyb.c_cflag & ~CS8) | CS7;
	Savettyb.c_oflag |= OPOST;
	Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
	return(ret);
}

restline()
{
	return( ioctl(0, TCSETA, &Savettyb) );
}
#else
/*
 * *** This is RT code which isn't used!!!
 *
 * #include <sgtty.h>
 * struct sgttyb Savettyb;
 * struct sg_spds {int sp_val, sp_name;} spds[] = {
 * 	{ 300,  B300},
 * 	{1200, B1200},
 * 	{4800, B4800},
 * 	{9600, B9600},
 * 	{0, 0} };
 * 
 * /***
 *  *	fixline(tty, spwant)	set speed/echo/mode...
 *  *	int tty, spwant;
 *  *
 *  *	return codes:  none
 *  */
 * 
 * fixline(tty, spwant)
 * int tty, spwant;
 * {
 * 	struct sgttyb ttbuf;
 * 	struct sg_spds *ps;
 * 	int speed = -1;
 * 	int ret;
 * 
 * 	if (spwant == 0) {
 * 		fixmode(tty);
 * 		return;
 * 	}
 * 	for (ps = spds; ps->sp_val; ps++)
 * 		if (ps->sp_val == spwant)
 * 			speed = ps->sp_name;
 * 	ASSERT(speed >= 0, "BAD SPEED", "", speed);
 * 	ioctl(tty, TIOCGETP, &ttbuf);
 * 	ttbuf.sg_flags =(ANYP|RAW);
 * 	ttbuf.sg_ispeed = ttbuf.sg_ospeed = speed;
 * 	ret = ioctl(tty, TIOCSETP, &ttbuf);
 * 	ASSERT(ret >= 0, "RETURN FROM STTY", "", ret);
 * 	ioctl(tty, TIOCHPCL, STBNULL);
 * 	ioctl(tty, TIOCEXCL, STBNULL);
 * 	return;
 * }
 * /***
 *  *	fixmode(tty)	fix kill/echo/raw on line
 *  *
 *  *	return codes:  none
 *  */
 * 
 * fixmode(tty)
 * int tty;
 * {
 * 	struct sgttyb ttbuf;
 * 	int ret;
 * 
 * 	ioctl(tty, TIOCGETP, &ttbuf);
 * 	ttbuf.sg_flags = (ANYP | RAW);
 * 	ret = ioctl(tty, TIOCSETP, &ttbuf);
 * 	ASSERT(ret >= 0, "STTY FAILED", "", ret);
 * 	ioctl(tty, TIOCEXCL, STBNULL);
 * 	return;
 * }
 * sethup(dcf)
 * {
 * 
 * 	ioctl(dcf, TIOCHPCL, STBNULL);
 * }
 * #define BSPEED B150
 * 
 * /***
 *  *	genbrk		send a break
 *  *
 *  *	return codes;  none
 *  */
 * 
 * genbrk(fn)
 * {
 * 	struct sgttyb ttbuf;
 * 	int ret, sospeed;
 * 	int	bnulls;
 * 
 * 	bnulls = 1;
 * 	ret = ioctl(fn, TIOCGETP, &ttbuf);
 * 	sospeed = ttbuf.sg_ospeed;
 * 	ttbuf.sg_ospeed = BSPEED;
 * 	ret = ioctl(fn, TIOCSETP, &ttbuf);
 * 	ret = write(fn, "\0\0\0\0\0\0\0\0\0\0\0\0", bnulls);
 * 	ASSERT(ret > 0, "BAD WRITE genbrk", "", ret);
 * 	ttbuf.sg_ospeed = sospeed;
 * 	ret = ioctl(fn, TIOCSETP, &ttbuf);
 * 	ret = write(fn, "@", 1);
 * 	ASSERT(ret > 0, "BAD WRITE genbrk", "", ret);
 * 	DEBUG(4, "sent BREAK nulls - %d\n", bnulls);
 * 	return;
 * }
 * setline()
 * {
 * }
 * savline()
 * {
 * 	int	ret;
 * 
 * 	ret = ioctl(0, TIOCGETP, &Savettyb);
 * 	Savettyb.sg_flags |= ECHO;
 * 	Savettyb.sg_flags &= ~RAW;
 * 	return(ret);
 * }
 * restline()
 * {
 * 
 * 		return(ioctl(0, TIOCSETP, &Savettyb));
 * }
#endif
