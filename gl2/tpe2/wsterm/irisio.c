/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
**		   	Encode and decode byte stream data
**
**			    Paul Haeberli - Aug 1983
*/
#include <gl.h>
#include "rpc.h"
#include "term.h"
#include "hostio.h"

#define rec8()		gethostchar()
#define send6(n)	puthostchar( ((n)&077) + ' ' )
#define dosync(i)	( ((i)&7) == 0 )

static long 	a_rvals[MAXARGS];
static char 	*a_arrayaddr[MAXARGS];
static short    arraycount;
static char 	localbuffer[1024];
static short    localbufferavailable = 1;

extern char *malloc();
extern void free();

#define PRINTABLE_ONLY

#ifdef PRINTABLE_ONLY
/* allow ^J (NL) and ^M (CR) through so that if wedged but host echos NL or
 * CR eventually enough characters will reach the terminal program to unwedge
 * it -- or so Paul says.
 */
static unsigned char bytetable[] = {
    0,0,0,0,0,0,0,0,	/* ^@ - ^G */
    0,0,1,0,0,1,0,0,	/* ^H - ^O */
    0,0,0,0,0,0,0,0,	/* ^P - ^W */
    0,0,0,0,0,0,0,0,	/* ^X - ^_ */
    1,1,1,1,1,1,1,1,	/* sp - '  */
    1,1,1,1,1,1,1,1,	/* (  - /  */
    1,1,1,1,1,1,1,1,	/* 0  - 7  */
    1,1,1,1,1,1,1,1,	/* 8  - ?  */
    1,1,1,1,1,1,1,1,	/* @  - G  */
    1,1,1,1,1,1,1,1,	/* H  - O  */
    1,1,1,1,1,1,1,1,	/* P  - W  */
    1,1,1,1,1,1,1,1,	/* X  - _  */
    1,1,1,1,1,1,1,1,	/* `  - g  */
    1,1,1,1,1,1,1,1,	/* h  - o  */
    1,1,1,1,1,1,1,1,	/* p  - w  */
    1,1,1,1,1,1,1,0,	/* x  - del */
    0,0,0,0,0,0,0,0,	/* ^@ - ^G */
    0,0,1,0,0,1,0,0,	/* ^H - ^O */
    0,0,0,0,0,0,0,0,	/* ^P - ^W */
    0,0,0,0,0,0,0,0,	/* ^X - ^_ */
    1,1,1,1,1,1,1,1,	/* sp - '  */
    1,1,1,1,1,1,1,1,	/* (  - /  */
    1,1,1,1,1,1,1,1,	/* 0  - 7  */
    1,1,1,1,1,1,1,1,	/* 8  - ?  */
    1,1,1,1,1,1,1,1,	/* @  - G  */
    1,1,1,1,1,1,1,1,	/* H  - O  */
    1,1,1,1,1,1,1,1,	/* P  - W  */
    1,1,1,1,1,1,1,1,	/* X  - _  */
    1,1,1,1,1,1,1,1,	/* `  - g  */
    1,1,1,1,1,1,1,1,	/* h  - o  */
    1,1,1,1,1,1,1,1,	/* p  - w  */
    1,1,1,1,1,1,1,0,	/* x  - del */
};
/*
**	rec6 - special weapons and tactics for ibm
**
*/
rec6()
{
    register int    onechar;

    while (bytetable[onechar = gethostchar()] == 0);
    return(onechar & 0x7f) - ' ';
}
#else
#define rec6()		( (gethostchar() &0x7f) - ' ' )
#endif

/*
**	receivef - uses the format string f to translate the incomming
**		   character stream into bytes, int and floats in a stack
** 		   frame.
*/
receivef(f, arg)
register char  *f;
register long  *arg;
{
    register short  retcount = 0;
    register long   c;
    register int    rv = 0;

    arraycount = 0;

    ++f;			/* skip the first format item - it is a return
				   value */

    while (*f) {
	if (*f == 'f' || *f == 'l') {/* optimize for these */
	    frecl((unsigned char *)arg);
	    f++;
	    arg++;
	}
	else {
	    switch (c = *f++) {
		case 'b': 
		case 'o': 
		    *arg++ = (long)recb();
		    break;
		case 's': 
		    *arg++ = (long)recs();
		    break;
		case 'c': 
		case 'a': 
		    if ((*arg++ = (long) reca()) == NULL)
			rv = -1;
		    break;
		case 'B': 
		case 'O': 
		case 'S': 
		case 'L': 
		case 'F': 
		    *arg++ = (long) (a_rvals + (retcount++));
		    break;
		default: 
		    errorm('w',
	"%s: bad format char: `%c' (0x%02x) -- check lib.prim",
			   getcmdname(curcmd), c, c);
		    rv = -1;
		    break;
	    }
	}
    }
    return rv;
}

/*
**	allocarray - allocate an array of bytes
**
*/
char *
alloca(nbytes)
unsigned nbytes;
{
    register char  *ptr;

    if (nbytes <= sizeof localbuffer && localbufferavailable) {
	ptr = localbuffer;
	localbufferavailable = 0;
    }
    else {
	if ((ptr = (char *) malloc(nbytes)) == NULL)
	    errorm('W',"%s: can't allocate temporary buffer",
	    	       getcmdname(curcmd));
	else
	    a_arrayaddr[arraycount++] = ptr;
    /* arraycount won't exceed MAXARGS -- see receivef() and irisinit() */
    }
    return ptr;
}

/*
**	freearrays - free any arrays malloced by allocarray
**
*/
freearrays()
{
    register short  i;

    localbufferavailable = 1;
    for (i = 0; i < arraycount; i++)
	free(a_arrayaddr[i]);
    arraycount = 0;
}

/*
**	reca - receive an array
**
*/
char   *reca()
{
    register long   i;
    register unsigned nlongs;
    register long  *aptr = NULL, *lptr;

    nlongs = ((recl() + 3) >> 2);
    if ((aptr = (long *) allocLa(nlongs)) != NULL) {
	lptr = aptr;
	for (i = 0; i < nlongs; i++) {
	    if (dosync(i))
		if ((gethostchar() & 0x7f) != AESC) {
		    errorm('w',"%s: error in array transport",
		    	       getcmdname(curcmd));
		    return NULL;
		}
	    frecl((unsigned char *)lptr++);
	}
    }
    return(char *) aptr;
}

/*
**	senda - send an array to the host
**
*/
senda(buffer, nlongs)
register long  *buffer;
register unsigned long nlongs;
{
    register long   i;
    register unsigned long  val;

    puthostchar(RESC);
    for (i = 0; i < nlongs; i++) {
	val = *buffer++;
	send6(val);
	send6(val >>= 6);
	send6(val >>= 6);
	send6(val >>= 6);
	send6(val >>= 6);
	send6(val >> 6);
	if (dosync(i)) {
	    puthostchar('\r');
	    flushhost();
	    if ((gethostchar() & 0x7f) != AESC) {
		errorm('w',"%s: error in array transport",
			   getcmdname(curcmd));
		return;
	    }
	}
    }
    puthostchar('\r');
    flushhost();
}

sendLs(array, n)
long   *array;
unsigned long n;
{
    sendL(n);
    senda(array, n);
}

sendFs(array, n)
long   *array;
unsigned long n;
{
    sendL(n);
    senda(array, n);
}

sendSs(array, n)
long   *array;
unsigned long n;
{
    sendL(n);
    senda(array, (n + 1) >> 1);
}

sendBs(array, n)
long   *array;
unsigned long n;
{
    sendL(n);
    senda(array, (n + 3) >> 2);
}


/*
**	recgcmd - receive a graphics command
**
*/
recgcmd()
{
    register    cmd;

    cmd = rec6();
    cmd |= rec6() << 6;
    return cmd;
}

/*
**	recb - receive a byte value
**
*/
char recb()
{
    register short  byte;

    if (fastmode)
	byte = rec8();
    else {
	byte = rec6();
	byte |= rec6() << 6;
    }
    return byte & 0xff;
}

/*
**	recs - receive a short value
**
*/
short recs()
{
    short   sval;
    register char  *ptr;

    if (fastmode) {
	ptr = (char *) & sval;
	*ptr++ = rec8();
	*ptr = rec8();
    }
    else {
	sval = rec6();
	sval |= rec6() << 6;
	sval |= rec6() << 12;
    }
    return sval;
}

/*
**	recl - receive a long value
**
*/
long    recl()
{
    long    lval;
    register unsigned char *ptr;

    if (fastmode) {
	ptr = (unsigned char *) & lval;
	*ptr++ = rec8();
	*ptr++ = rec8();
	*ptr++ = rec8();
	*ptr = rec8();
    }
    else {
	lval = rec6();
	lval |= rec6() << 6;
	lval |= rec6() << 12;
	lval |= rec6() << 18;
	lval |= rec6() << 24;
	lval |= rec6() << 30;
    }
    return lval;
}

/*
**	frecl - faster receive long.  assumes rec8 does the following:
**		#define gethostchar() 	\
**				(--rc >= 0 ? *rp++ : fillhostbuffer())
**
*/
frecl(ptr)
register unsigned char *ptr;
{
    register unsigned char *inptr;

    if (fastmode && rc >= 4) {
	inptr = rp;
	*ptr++ = *inptr++;
	*ptr++ = *inptr++;
	*ptr++ = *inptr++;
	*ptr = *inptr++;
	rp = inptr;
	rc -= 4;
    }
    else
	*(long *) ptr = recl();
}

/*
**	senddata - uses the format string f to determine what values are
**		   expected by the host.
*/
senddata(f)
register char  *f;
{
    register short  retcount;
    register short  sentsomething = 0;

    switch (*f++) {
	case 'B': 
#ifdef GL1
	case 'O': 
#endif GL1
	    sentsomething = 1;
	    sendB((unsigned char) a_retval);
	    break;
	case 'S': 
	    sentsomething = 1;
	    sendS((unsigned short) a_retval);
	    break;
	case 'L': 
#ifndef GL1
	case 'O': 
#endif not GL1
	    sentsomething = 1;
	    sendL((unsigned long) a_retval);
	    break;
	case 'F': 
	    sentsomething = 1;
	    sendL((unsigned long) a_retval);
	    break;
    }

    retcount = 0;
    while (*f) {
	switch (*f++) {
	    case 'B': 
#ifdef GL1
	    case 'O': 
#endif GL1
		sentsomething = 1;
		sendB(*(unsigned char *) (a_rvals + retcount++));
		break;
	    case 'S': 
		sentsomething = 1;
		sendS(*(unsigned short *) (a_rvals + retcount++));
		break;
	    case 'L': 
#ifndef GL1
	    case 'O': 
#endif not GL1
		sentsomething = 1;
		sendL(*(unsigned long *) (a_rvals + retcount++));
		break;
	    case 'F': 
		sentsomething = 1;
		sendL(*(unsigned long *) (a_rvals + retcount++));
		break;
	}
    }
    if (sentsomething) {
	puthostchar('\r');
	flushhost();
    }
}

/*
**	sendB - send a byte value
**
*/
sendB(val)
register unsigned char  val;
{
    puthostchar(RESC);
    send6(val);
    send6(val >> 6);
}

/*
**	sendS - send a short value
**
*/
sendS(val)
register unsigned short val;
{
    puthostchar(RESC);
    send6(val);
    send6(val >>= 6);
    send6(val >> 6);
}

/*
**	sendL - send a long value
**
*/
sendL(val)
register unsigned long  val;
{
    puthostchar(RESC);
    send6(val);
    send6(val >>= 6);
    send6(val >>= 6);
    send6(val >>= 6);
    send6(val >>= 6);
    send6(val >> 6);
}

#ifndef GL1
/*
**	xsetslowcom - set the communication mode to slow (6 bits per char)
**
*/
Boolean xsetslowcom()
{
    fastmode = 0;
    if (hosttype == SERIAL_TYPE)
	condline();
    return (TRUE);
}

/*
**	xsetfastcom - set the communication mode to fast (binary).
**		      fail if serial and -y option or if TCP/IP.
**
*/
Boolean xsetfastcom()
{
    if ((hosttype == SERIAL_TYPE && yflag) || (hosttype == TCP_TYPE))
	return (FALSE);
    fastmode = 1;
    if (hosttype == SERIAL_TYPE)
	condline();
    return (TRUE);
}
#endif not GL1

/*
**	zzsetslowcom - set the communication mode to slow (6 bits per char)
**		      (old version which doesn't return a value -- kept in
**		      dispatch table to be compatible with old remote libs;
**		      GL1 lib.prim is unchanged)
**
*/
#ifdef GL1
void xsetslowcom()
#else not GL1
void zzsetslowcom()
#endif not GL1
{
    fastmode = 0;
}

/*
**	zzsetfastcom - set the communication mode to fast (binary)
**		      (old version which doesn't return a value -- kept in
**		      dispatch table to be compatible with old remote libs;
**		      GL1 lib.prim is unchanged)
**
*/
#ifdef GL1
void xsetfastcom()
#else not GL1
void zzsetfastcom()
#endif not GL1
{
    if (yflag)
	errorm('f', "can't use fastmode with -y option");
    fastmode = 1;
}
/*
**	gversion - return a status number indicating that we're running 
**		  iris terminal emulator
**
*/
gversion()
{
    return 200;
}

/*
**	turndelay - delay for a bit for line turnaround.  This is
**		    unimplemented in UNIX-land
**
*/
turndelay()
{
}


/*
**	tadelay - set turnaround delay.  This is currently a NOP in 
**		  UNIX-land, so it's used to open a new log file under
**		  host control.
**
*/
tadelay(d)
short   d;
{
    static int  logfnum = 0;

    if (dflag[1]) {
	if (d < 0)
	    logfnum++;
	else
	    logfnum = d;
	sprintf(logfile, "LOGFILE.%d", logfnum);
	openlogfile();
    }
}

/*
**	openlogfile - opens a new logfile, closing the existing one, if one
**		      is open.
**
*/
openlogfile(mode)
char *mode;
{
    closelogfile();
    if ((lf = fopen(logfile, mode)) == NULL)
	errorm('F',"can't open %s", logfile);
}


/*
**	closelogfile - closes the logfile.
**
*/
closelogfile()
{
    if (lf)
	fclose(lf);
}


/*
** 	logrecv - write the arguments received to the log file.  NOTE: the 
**		  arguments to the special routines in ret.c are not logged.
**
*/
logrecv(f, arg)
register char  *f;
register long  *arg;
{
    /* skip the first format item - it is a return value */
    while (*++f) {
	switch (*f) {
	    case 'o': 
	    case 'l': 
	    case 's': 
	    case 'b': 
		fprintf(lf, "%c=%d ", *f, *arg);
		break;
	    case 'f': 
		fprintf(lf, "f=%g ", *arg);
		break;
	    case 'a': 
		/* gross hack to get FORTRAN stings to print -- assumes
		 * cmd numbers won't change and that the string length
		 * is alway the next argument
		 */
		switch (curcmd) {
		    case 313:	/* charst */
		    case 314:	/* strwid */
		    case 336:	/* getpor */
		    case 352:	/* wintit */
		    case 354:	/* iftpse */
		    case 356:	/* captur */
		    case 358:	/* rcaptu */
			fprintf(lf, "c=\"%.*s\" ", *(arg+1), *arg);
			break;
		    case 325:	/* dbtext */
			fprintf(lf, "c=\"%.8s\" ", *(arg+1), *arg);
			break;
		    default:
			fprintf(lf, "a=array ");
			break;
		}
		break;
	    case 'c': 
		fprintf(lf, "c=\"%s\" ", *arg);
		break;
	}
    arg++;
    }
}

/*
** 	logsend - write the arguments sent to the log file.  NOTE: the 
**		  arguments to the special routines in ret.c are not logged.
**
*/
logsend(fmt)
char  *fmt;
{
    register char *f = fmt;
    register long val;
    register long retcount = 0;
    
    /* put return value at end of line */
    while (*++f) {
	switch (*f) {
	    case 'B': 
#ifdef GL1
	    case 'O': 
#endif GL1
		val = *(char *) (a_rvals + retcount++);
		goto printit;

	    case 'S': 
		val = *(short *) (a_rvals + retcount++);
		goto printit;

	    case 'L': 
#ifndef GL1
	    case 'O': 
#endif not GL1
		val = *(long *) (a_rvals + retcount++);
	printit:
		fprintf(lf, "%c=%d ", *f, val);
		break;
	
	    case 'F': 
		val = *(long *) (a_rvals + retcount++);
		fprintf(lf, "F=%g ", val);
		break;
	}
    }

    fprintf(lf, ") ");

    /* put return value at end of line */
    switch (*fmt) {
	case 'B': 
	case 'O': 
	case 'S': 
	case 'L': 
	    fprintf(lf, "return %c=%d ", *fmt, a_retval);
	    break;
	case 'F': 
	    fprintf(lf, "return F=%g ", a_retval);
	    break;
    }
    fprintf(lf, "\n");
    fflush(lf);
}
