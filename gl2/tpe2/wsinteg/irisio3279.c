/*
**		   	Encode and decode byte stream data
**
**			    Paul Haeberli - Aug 1983
*/
#include "rpc.h"
#include "term.h"
#include <stdio.h>
#include <ctype.h>
#include "hostio.h"
#include <sys/types.h>
#include "pxw.h"

#define rec8()		gethostchar()
#define send6(n)	puthostchar( ((n)&077) + ' ' )
#define dosync(i)	( ((i)&7) == 0 )
#define STACKLONGS 276
#define MARKER	   0xBADBADAD	    /* if this is overwritten, then some
				     * GL routine's stack has exceeded
				     * the bounds of args[]
				     */

unsigned	curcmd;
extern int	bogus();
extern char	*malloc();
extern void	free();

static char *cwa_temp;
static char *cwa_newsp;
static  FUNPTR cwa_func;
static long 	args[STACKLONGS] = { MARKER };   
				    /* Must be big enough to accomodate the
				     * maximum stack growth of any GL routine.
				     * As of 3.3.1, capture() is the routine
				     * which causes the most stack growth.
				     */

long    a_retval;

static short    fastmode = 0;	/* if 1 then in fast lane */
static long	a_rvals[MAXARGS];
static char	*a_arrayaddr[MAXARGS];
static short    arraycount;
static char	localbuffer[1024];
static short    localbufferavailable = 1;

#define INPUT_CAPS_ONLY

#ifdef INPUT_CAPS_ONLY
/* allow ^J (NL) through so that if wedged but host echos NL, eventually
 * enough characters will reach the terminal program to unwedge it --
 * or so Paul says
 */
static unsigned char    bytetable[]
= {
    0, 0, 0, 0, 0, 0, 0, 0,	/* ^@ - ^G */
    0, 0, 1, 0, 0, 0, 0, 0,	/* ^H - ^O */
    0, 0, 0, 0, 0, 0, 0, 0,	/* ^P - ^W */
    0, 0, 0, 0, 0, 0, 0, 0,	/* ^X - ^_ */
    1, 1, 1, 1, 1, 1, 1, 1,	/* sp - '  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* (  - /  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 0  - 7  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 8  - ?  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* @  - G  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* H  - O  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* P  - W  */
    1, 1, 1, 1, 1, 1, 1, 1,	/* X  - _  */
    0, 0, 0, 0, 0, 0, 0, 0,	/* `  - g  */
    0, 0, 0, 0, 0, 0, 0, 0,	/* h  - o  */
    0, 0, 0, 0, 0, 0, 0, 0,	/* p  - w  */
    0, 0, 0, 0, 0, 0, 0, 0,	/* x  - del */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/*
**	Turn on/off the tracing of this module
*/
tr_iris(flag)
{
	trace = flag;
}


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
/*		    fprintf(stderr,
	    "t3279: bad format char: `%c' (0x%02x) -- check lib.prim\n\r",
    			    c, c);*/
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
/*	    fprintf(stderr,
		    "t3279: alloca: can't allocate receive buffer\n\r");*/
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
/*		    fprintf(stderr,
			    "t3279: reca: error in array transport\n\r");*/
#ifdef DEBUG
		    DT("reca error ");
#endif /* DEBUG */
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
register unsigned long *buffer;
register unsigned long nlongs;
{
    register long   i;
    register unsigned long  val;

#ifdef DEBUG
    DT("senda d %d ",nlongs);
#endif /* DEBUG */
    puthostchar(RESC);
    for (i = 0; i < nlongs; i++) {
	val = *buffer++;
#ifdef DEBUG
	DT("L%x-",val);
#endif /* DEBUG */
	send6(val);
	send6(val >>= 6);
	send6(val >>= 6);
	send6(val >>= 6);
	send6(val >>= 6);
	send6(val >> 6);
/*	if (dosync(i)) {
	    puthostchar('\r');
	    flushhost();
	    if ((gethostchar() & 0x7f) != AESC) {
		fprintf(stderr,
			"t3279: senda: error in array transport \n\r");
		return;
	    }
	}*/
    }
#ifdef DEBUG
    DT("\n");
#endif /* DEBUG */
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
recb()
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
recs()
{
    unsigned short   sval;
    register u_char  *ptr;

    if (fastmode) {
	ptr = (u_char *) & sval;
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
long recl()
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
**		#define gethostchar() 	
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
	    sentsomething = 1;
	    sendB((u_char) a_retval);
	    break;
	case 'S': 
	    sentsomething = 1;
	    sendS((u_short) a_retval);
	    break;
	case 'L': 
	case 'O': 
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
		sentsomething = 1;
		sendB(*(u_char *) (a_rvals + retcount++));
		break;
	    case 'S': 
		sentsomething = 1;
		sendS(*(u_short *) (a_rvals + retcount++));
		break;
	    case 'L': 
	    case 'O': 
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
#ifdef DEBUG
    DT("B%x-",val);
#endif /* DEBUG */
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
#ifdef DEBUG
    DT("S%x-",val);
#endif /* DEBUG */
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
#ifdef DEBUG
    DT("L%x-",val);
#endif /* DEBUG */
    puthostchar(RESC);
    send6(val);
    send6(val >>= 6);
    send6(val >>= 6);
    send6(val >>= 6);
    send6(val >>= 6);
    send6(val >> 6);
}

turndelay()
{
}


/*
**	xsetslowcom - set the communication mode to slow (6 bits per char)
**
*/
void xsetslowcom()
{
    fastmode = 0;
}

/*
**	xsetfastcom - set the communication mode to fast (binary)
**
*/
void xsetfastcom()
{
    fastmode = 1;
}

/*
**	gversion - return a status number indicating that we're running 
**		  iris term program.
**
*/
gversion()
{
    return 200;
}

/*
**	tadelay - set turnaround delay.
**
**	This is normally a NOP in UNIX-land, and is used to open a new log
**	file.
**
*/
tadelay(d)
short   d;
{
    char    buf[20];
    static int  logfnum = 0;

    if (dflag[1]) {
	if (lf)
	    fclose(lf);
	gcmdcnt = 0;
	if (d < 0)
	    logfnum++;
	else
	    logfnum = d;
	sprintf(buf, "LOGFILE.%d", logfnum);
	if ((lf = fopen(buf, "w")) == NULL)
	    oops("t3279: can't open %s\n\r", buf);
	printf("opened LOGFILE.%d ",logfnum);
    } else {
	if (lf)
	    fclose(lf);
	lf = 0;
	printf("closed LOGFILE.%d ",logfnum);
    }

}


/*
**	doprimitive - get a graphics command code and call the primitive
**
*/
doprimitive()
{
    register unsigned cmd;
    register dispatchEntry *prim;
    register char *newsp;

    curcmd = cmd = recgcmd();
    if (lf) {
	char   *cp;
	switchtographics();
	fprintf(lf, "%d: ", gcmdcnt++);
	cp = getcmdname(cmd);
	if (!isdigit(*cp) && strcmp(cp,"bogus"))
	    fprintf(lf, "%s ( ", cp);
	else
	    fprintf(lf, "gcmd=%d ( ", cmd);
/*	if ((cp = getcmdname(cmd)) && strcmp(cp,"bogus"))
	    fprintf(lf, "%s ( ", cp);
	else
	    fprintf(lf, "gcmd=%d ( ", cmd);*/
	fflush(lf);
    }

    if (cmd >= maxcom) {
	errorm('w', "command code out of range: %d", cmd);
	return;
    }
    prim = &dispatch[cmd];
    newsp = (char *)(&args[STACKLONGS - 1]) - (prim->framesize);
    if (receivef(prim->format, newsp) >= 0) {
	if (lf)
	    logrecv(prim->format, newsp);
	switchtographics();
#ifdef GL1
	if (prim->func == 0) {
	    newsp -= 4;
	    *((long *) newsp) = prim->token << 2;
	    newsp -= 4;
	    *((long *) newsp) = prim->arg;
	    cwa_newsp = newsp;

	    asm("movl sp, _cwa_temp");/* sorry kipp and bruce */
	    asm("movl _cwa_newsp, sp");
	    a_retval = gl_intcmd();
	    asm("movl _cwa_temp, sp");
	    gflush();
	}
	else
#endif GL1

	if (prim->func == bogus) {
	    errorm('w', "bogus command %d - using the right library?", cmd);
	}
	else {
	    cwa_newsp = newsp;

	    asm("movl sp, _cwa_temp");/* sorry kipp and bruce */
	    asm("movl _cwa_newsp, sp");

	    a_retval = (prim->func) ();
	    asm("movl _cwa_temp, sp");
	    if (args[0] != MARKER) {
		errorm('f', 
		       "%s exceeded stack depth limit - increase STACKLONGS",
		       getcmdname());
	    }


#ifdef GL1
	    /* 
	     * first couple of cmds aren''t really graphics, e.g setfastcom
	     */
	    if (graphinited)
		gflush();
	    senddata(prim->format);
#else  not GL1
	    if (prim->returnsdata)
		senddata(prim->format);
#endif not GL1
	}
	if (lf)
	    logsend(prim->format);
    }	     
    else if (lf) {
	fprintf(lf, "receivef error )\n");
	fflush(lf);
    }

    freearrays();
}


/*
 * NOTE: the arguments to the special routines in ret.c are not
 * logged
 */

logrecv(f, arg)
register char  *f;
register unsigned long  *arg;
{
    register unsigned long *p;
    register long acountr;
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
	    case 'd': /* my array hack */
		fprintf(lf, "a=array ");
		p = (unsigned long *)*arg;
		acountr = *(arg+1)>=18 ? 18 : *(arg+1);
		while (acountr-- )
			fprintf(lf, "%08x ", *p++);
		break;
	    case 'a': 
		/* gross hack to get FORTRAN strings to print -- assumes
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

logsend(fmt, arg)
char  *fmt;
register long  *arg;
{
    register char *f = fmt;
    register unsigned long val;
    register long retcount = 0;
    
    /* put return value at end of line */
    while (*++f) {
	switch (*f) {
	    case 'B': 
		val = *(u_char *) (a_rvals + retcount++);
		goto printit;

	    case 'S': 
		val = *(u_short *) (a_rvals + retcount++);
		goto printit;

	    case 'L': 
	    case 'O': 
		val = *(unsigned long *) (a_rvals + retcount++);
	printit:
		fprintf(lf, "%c=%d ", *f, val);
		break;
	
	    case 'F': 
		val = *(unsigned long *) (a_rvals + retcount++);
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
