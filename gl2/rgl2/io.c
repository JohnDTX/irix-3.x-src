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
 *  This file, io.c, contains C i/o primitives for remote graphics library.
 *  It is specific to UNIX systems.  There are various
 *  compile options that allow use on various configurations.  These
 *  options are selected by defining one or more of the following
 *  (one each of the first two sets MUST be defined):
 *
 *	UNIX4_2 for systems running Berkeley UNIX bsd4.2
 *	SYSTEM5 for systems running UNIX System V
 *
 *	DEC_INTEL for systems that put LSByte in lowest memory
 *	MOT_IBM   for systems that put MSByte in lowest memory
 *	VAX       causes DEC11 and DEC_INTEL to be defined
 *	DEC11 for DEC VAX's, etc., with non-IEEE floating point format;
 *		also causes DEC_INTEL to be defined
 *	MC68000 causes MOT_IBM to be defined
 *
 *	IO_DEBUG, if a human readable printout of all host/terminal
 *		io is to be put in the file 'protsave' each time a
 *		program using a library made from this io.c is run
 *
 *	TCP_ONLY if XNS has not been installed on the system, i.e.,
 *		if <xns/Xnsioctl.h> doesn't exist.
 */

/*
 * Brief description of the philosophy behind the i/o library:
 *
 *
 *             HOST COMPUTER            
 * +------------------------------------+
 * |	user's application program	|
 * +------------------------------------+
 * |	lib.x (e.g, lib.c or lib.f)	| set of auto-generated stubs 
 * +------------------------------------+
 * |		io.c or io.f		| routines that really do the i/o
 * +------------------------------------+
 *		    ^
 *		    |
 *		    |			  serial line or ethernet
 *		    |
 *		    v
 *
 *	 	  IRIS
 *	+-----------------------+
 *	|  graphics software    |
 *	+-----------------------+
 *
 *     The user's application program calls routines found in lib.c, which
 * in turn call routines in io.c.
 *     Commands that are to be passed to the IRIS are buffered. When the
 * buffer gets full; or when (flushg, gflush, gexit, greset, gfinis,
 * ginit) are called; and occasionally when (recOs, recBs, recFs, recLs,
 * recSs) are called, the buffer is emptied.
 *     All tokens are passed as a byte stream, but in even multiples of 4
 * bytes (i.e. a longword). If a character string, for example, is not an
 * even multiple of four, extra bytes are tacked onto the end when it is
 * put into the buffer. These extra bytes are ignored when the IRIS
 * receives them, as a count is also passed.
 *     When arrays of (bytes, characters, longs, floats,...) are sent to
 * the IRIS, they are sent in blocks. Blocks start with a well-known
 * escape character. If running in fastmode (see next paragraph), 8
 * longwords of information follow. If slowmode, it is still 8 longwords
 * of information, but because it takes 6 bytes to "encrypt" a longword,
 * there are 6*8 = 48 bytes that follow. In either case, if it is the end
 * of the array, then fewer bytes may follow.
 *     There are two transmission modes: fastmode and slowmode. Fastmode
 * sends 8 bit quantities (high byte to low byte), while slowmode sends 6
 * bit quantities starting with the low byte. Note that slowmode also adds
 * the ascii value of "blank" to the 6 bit entity, so as to make it a
 * printable (and hence traceable) character.
 *     Typically slowmode is used for a serial line connection, and
 * fastmode is used for an ethernet connection using the SGI ethernet
 * software.
 */


#include <stdio.h>
#include <signal.h>
#ifdef UNIX4_2
#include <sgtty.h>
#endif
#ifdef SYSTEM5
#include <termio.h>
#endif
#include <setjmp.h>
#ifndef TCP_ONLY */
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>
#endif
#include "rpc.h"
#include "decl.h"

       /**************************
	*                        *
	*  Definitions & macros  *
	*                        *
	**************************/

       /*
	* If the token 'VAX' is unusable (as under VMS), comment out the
	* 3 lines below and define 'DEC11' when desired instead of 'VAX'.
	*/
#ifdef VAX
#define DEC11		1
#endif

       /*
	*  DEC11 machines use DEC_INTEL byteswapping, so automatically define
	*  it
	*/
#ifdef DEC11
#define DEC_INTEL	1
#endif

       /*
	*  System specific defines for machines with LSByte lowest in memory
	*
	*  'wdelem' stands for 'word element'
	*/
#ifdef DEC_INTEL
#define lwdelem0	3	/* msbyte */
#define lwdelem1	2
#define lwdelem2	1
#define lwdelem3	0	/* lsbyte */

#define swdelem0	1	/* msshort */
#define swdelem1	0	/* lsshort */
#endif

       /*
	*  MC68000 machines use MOT_IBM byteswapping, so automatically define
	*  it
	*/
#ifdef MC68000
#define MOT_IBM		1
#endif

       /*
	*  System specific defines for machines with MSByte lowest in memory
	*/
#ifdef MOT_IBM
#define lwdelem0	0	/* msbyte */
#define lwdelem1	1
#define lwdelem2	2
#define lwdelem3	3	/* lsbyte */

#define swdelem0	0	/* msshort */
#define swdelem1	1	/* lsshort */
#endif
       /*
	*  End of system specific defines for byteswapping
	*/


unsigned long recO();	/* receive bOolean */
unsigned long recB();	/* receive Byte */
float recF();		/* receive Float */
unsigned long recS();	/* receive Short */
unsigned long recL();	/* receive Long */

static short fastmode = 0;
static short netinited = 0;
static short haveether = 0;
static short offbuffend = 0;

       /* 
	* OUTBUFFSIZE is the size in bytes of the output buffer for
	* IRIS tokens If it is large, you will have fewer writes to
	* deal with, but real time stuff may appear jerky.  If it is
	* small, real time appearance will be better, but the system
	* will have to perform more writes, and hence "work harder".
	* The point is, you may want to play with this value and get
	* the right value for your application!
        */
#define OUTBUFFSIZE		1024
	/* INBUFSIZE should remain > 100 */
#define INBUFFSIZE		256
	/* longest non-array command/data group of bytes sent or received */
#define LONGESTBYTEGROUP 	50


	/* Output buffer for holding the IRIS byte stream */
static char outbuff[OUTBUFFSIZE];
	/* Pointer into the output buffer for inserting the next character */
static char *wp = outbuff;
	/* High water mark.  When reached, means the output buffer is full */
static char *highwp = outbuff+OUTBUFFSIZE-8;
	/* Indicator that putting another command's worth of info in the
	   output buffer may cause an overflow */
static char *checkwp = outbuff+OUTBUFFSIZE-LONGESTBYTEGROUP;
	/* Input buffer for holding the IRIS byte stream */
static char inbuff[INBUFFSIZE];
	/* Read count (set by fillbuff() ) */
static int rc = 0;
	/* Pointer into the input buffer for reading the next character */
static unsigned char *rp;


static int irisfile;
#ifdef UNIX4_2
static struct sgttyb ttydat, origttydat;
#endif
#ifdef SYSTEM5
static struct termio ttydat, origttydat;
#endif


       /*
	*  MACROS
	*/
	/* receive 6 bitsworth of information from the IRIS */
#define rec6()		( (getgchar() - ' ') & 077)
	/* send 6 bitsworth of info to the IRIS */
#define send6(n)	putgchar( ((n) & 077) + ' ')
	/* send 8 bitsworth (fastmode) of info to the IRIS */
#define send8(n)	putgchar(n)
	/* See if it's time to send another sync character */
#define dosync(i)	( ((i) & 7) == 0)

#define addchar(n)	(*wp++ = (n) )		
	/* See if output buffer is full, and if so, flush the buffer */
#define fullcheck();	{ if (wp > highwp) { flushg(); offbuffend++; } }

       /************************************************
	*                                              *
	*  Calls used only by this io.c module itself  *
	*                                              *
	************************************************/

#ifdef IO_DEBUG
#define NONE 0
#define READER 1
#define WRITER 2
static isopen = 0, current = NONE, column = 0;
FILE *fopen(), *fp;


char getgchar()
{
    unsigned char ch;
    ch = (--rc >= 0 ? *rp++ : fillbuff() );
    if (current == WRITER)
    {
	fprintf(fp, "end");
	fprintf(fp, "\nGot from iris:\n");
	current = READER;
	column = 0;
    }
    print6char(ch);
    return(ch);
}


print6char(ch)
unsigned char ch;
{
    if (ch == 0)
	fprintf(fp, "  \\0 ");
    else if ((ch < ' ') || (ch > '~'))
	fprintf(fp, " x%02x ", ch);
    else
	fprintf(fp, "%c=%02x ", ch, ch);
    if (++column == 16)
    {
	fprintf(fp, "\n");
	column = 0;
    }
    fflush(fp);
}
#else
#define getgchar() 	(--rc >= 0 ? *rp++ : fillbuff() )
#endif


       /*
	*  Paul Haeberli - Sept 1983
	*/
#ifdef IO_DEBUG
putgchar(onechar)
char onechar;
{
    if (isopen == 0)
    {
	isopen = 1;
	fp = fopen("protsave", "w");
    }
    if (current == READER || current == NONE)
    {
	if (current != NONE)
	    fprintf(fp, "end");
	fprintf(fp, "\nSent to iris:\n");
	current = WRITER;
	column = 0;
    }
    print6char(onechar);
    *wp++ = onechar;
    if (wp > highwp)
    {
	flushg();
	offbuffend++;
    }
}
#else
#define putgchar(n)	addchar(n)
#endif


       /*
	*  outcheck - see if we are within LONGESTBYTEGROUP of the the
	*	end of the buffer.  If so, flush output.
	*/
outcheck()
{
    if (wp > checkwp)
	flushg();
    offbuffend = 0;
}


       /*
	*  arrayflush - only flush when needed after array transfer  
	*
	*/
arrayflush()
{
    if (offbuffend)
    {
	flushg();
        offbuffend = 0;
    }
}


       /*
	*  fillbuff - fill the input buffer
	*
	*/
fillbuff()
{
    do
    {
	rc = read(irisfile, inbuff, INBUFFSIZE);
    } while (rc <= 0);
    rc--;
    rp = (unsigned char *)inbuff;
    return *rp++;
}


#ifndef TCP_ONLY

netwrite(fd, buf, cc)
int fd;
char *buf;
register cc;
{
    struct xnsio io;

    io.addr = buf;
    io.count = cc;
    io.dtype = 0;
    io.control = 0;
    ioctl(fd, NXWRITE, &io);
    return(io.count);
}


#if 0 	/* this routine is never used */
netread(fd, buf, cc)
int fd;
char *buf;
register cc;
{
    struct xnsio io;

    io.addr = buf;
    io.count = cc;
    io.dtype = 0;
    io.control = 0;
    ioctl(fd, NXREAD, &io);
    return(io.count);
}
#endif

#endif not TCP_ONLY


cleanexit()
{
    ttyrestore();
    exit(0);
}


netinit()
{
    register short i;
    char *ifp, *getenv();
    int fast;

    netinited = 1;		   /* careful - the order is important! */
    if ( (ifp = getenv("IRISFILE")) == NULL)
	ifp = "/dev/tty";
    if ( (irisfile = open(ifp, 2)) == -1)
    {
	fprintf(stderr, "error opening %s\n", ifp);
	exit(1);
    }

#ifdef TCP_ONLY
    fast = 0;
#else  not TCP_ONLY
	/* see if we're on an XNS net */
    if ( ioctl(irisfile, NXIOSLOW, 0) == -1 && (strcmp(ifp, "/dev/tty") == 0) )
    	fast = 0; 		/* serial line	*/
    else
    {
	haveether = (strcmp(ifp, "/dev/tty") == 0);
	fast = 1;
    }
#endif not TCP_ONLY

    ttysave();
    signal(SIGQUIT, cleanexit);
    signal(SIGINT, cleanexit);
    signal(SIGBUS, cleanexit);
    signal(SIGTERM, cleanexit);

    for (i=0; i<LONGESTBYTEGROUP; i++)	/* sure to get out of graphics mode */
    {
	putgchar(0);
	fullcheck();
    }
    if (fast)
	(void)setfastcom();
    else
	(void)setslowcom();
}


       /*
	*  ttysave - save the state of the tty
	*
	*/
ttysave()
{
#ifdef UNIX4_2
    short t_local = LLITOUT;

    gtty(irisfile, &origttydat);
    gtty(irisfile, &ttydat);
    if (!haveether)
        ioctl(irisfile, TIOCLBIS, &t_local);	/* set literal output mode */
#endif
#ifdef SYSTEM5
    ioctl(irisfile, TCGETA, &origttydat);
    ioctl(irisfile, TCGETA, &ttydat);
#endif
}


       /*
	*  ttyrestore - restore the state of the tty
	*
	*/
ttyrestore()
{
#ifdef UNIX4_2
    short t_local = LLITOUT;

    stty(irisfile, &origttydat);
    if (!haveether)
        ioctl(irisfile, TIOCLBIC, &t_local);	/* clr literal output mode */
#endif
#ifdef SYSTEM5
    ioctl(irisfile, TCSETA, &origttydat);
#endif
}


#ifdef DEC11
       /*
	*  F2IEEE - convert from DEC floating point format to IEEE
	*	format type cast as an unsigned long to send.
	*  Returns zero on underflow (instead of denormalized IEEE).  
	*
	* 	Charles (Herb) Kuta - Aug 1983
	*/
unsigned long F2IEEE(fn)
float fn;
{
    register unsigned long n;
    						/* if characteristic <= 2 */
    if ( ((n = *(long *)&fn) & (0xff << 7)) <= (0x2 << 7) )   
	return 0;
    else
    {
        n -= 0x2 << 7;
        return ( (n >> 16) | (n << 16) );
    }
}


       /*
	*  IEEE2F - convert from IEEE floating point format received
	*	as an unsigned long, to DEC format.
	*  Returns zero for denormalized IEEE and DECmax on overflow 
	*	(including IEEE infinity).
	*
	*	Charles (Herb) Kuta - Aug 1983
	*/
float IEEE2F(n)
register unsigned long n;
{
    register unsigned long characteristic;
    unsigned long nn;

    n = ((n >> 16) & 0xffff) | (n << 16);
    if ((characteristic = n & (0xff << 7)) == 0)
    	return 0.0;
    else if (characteristic >= (0xfe << 7))
    	nn = (n < 0) ? 0xffffffff : 0xffff7fff;
    else
    	nn = n + (0x2 << 7);
    return *(float *)&nn;
}
#else  /*  if DEC11 not defined, use IEEE floating point format  */

       /*
	*  F2IEEE - type cast IEEE floating point format to send
	*	as an unsigned long.
	*/
unsigned long F2IEEE(value)
float value;
{
    return *(unsigned long *)&value;
}


       /*
	*  F2IEEE - type cast IEEE floating point format received
	*	as an unsigned long.
	*/
float IEEE2F(value)
unsigned long value;
{
    return *(float *)&value;
}
#endif


       /*****************************************
	*                                       *
	*  lib.c calls requiring data transfer  *
	*                                       *
	*****************************************/

       /*
	*  gcmd - send the graphics escape and the specified command code
	*	The values passed are (for the most part) compiled into lib.c.
	*/
gcmd(comcode)
register short comcode;
{
    if (!netinited)
	netinit();
    outcheck();
    putgchar(TESC);
    send6(comcode);
    send6(comcode >> 6);
}


       /*
	*  sendo - send a bOolean
	*
	*/
sendo(value)
unsigned char value;
{
    sendb((unsigned char)value);
}


       /*
	*  sendos - send an array of bOoleans
	*
	*/
sendos(values, nvals)
char values[];
unsigned long nvals;
{
    sendbs(values, nvals);
}


       /*
	*  sendb - send a Byte 
	*
	*/
sendb(value)
register unsigned char value;
{
    if (fastmode)
	send8(value);
    else
    {
        send6(value);
        send6(value >> 6);
    }
    fullcheck();
}


       /*
	*  sendbs - send an array of Bytes
	*
	*/
sendbs(values, nvals)
register char values[];
long nvals;
{
    register long i, nlongs;
    long along; 
    register char *ptr = (char *)&along;

    nlongs = (nvals + 3) >> 2;
    sendl(nvals);
    if (fastmode)
    {
	for (i=0; i<nlongs; i++)
	{
	    if (dosync(i) )
		putgchar(AESC);
	    send8(*values++);
	    send8(*values++);
	    send8(*values++);
	    send8(*values++);
	    fullcheck();
	}
    } else
    {
	for (i=0; i<nlongs; i++)
	{
	    if (dosync(i) )
		putgchar(AESC);
	    ptr[lwdelem0] = *values++;	
	    ptr[lwdelem1] = *values++;	
	    ptr[lwdelem2] = *values++;	
	    ptr[lwdelem3] = *values++;	
	    sendl(along);
        }
    }
    arrayflush();
}


       /*
	*  sendqs - send an array of Fontchars
	*
	*/
sendqs(values, nvals)
register unsigned char values[];
long nvals;
{
    register long i;
    unsigned long nlongs, along; 
    register unsigned char *bptr = (unsigned char *)&along;
    register unsigned short *sptr = (unsigned short *)&along;

    sendl(nvals << 3);
    nlongs = nvals << 1;
    for (i=0; i<nlongs; i++)
    {
	if (dosync(i) )
            putgchar(AESC);
	sptr[swdelem0] = values[swdelem1] + (values[swdelem0] << 8);
	values++;
	values++;
	bptr[lwdelem2] = *values++;
	bptr[lwdelem3] = *values++;
	sendl(along);
	i++;
	if (dosync(i) )
            putgchar(AESC);
	bptr[lwdelem0] = *values++;	
	bptr[lwdelem1] = *values++;	
	sptr[swdelem1] = values[swdelem1] + (values[swdelem0] << 8);
	values++;
	values++;
	sendl(along);
    }
    arrayflush();
}


       /*
	*  sendc - send a string
	*
	*/
sendc(str)
register char *str;
{
    sendbs(str, strlen(str) + 1);
}


       /*
	*  sends - send a Short
	*
	*/
sends(value) 
register unsigned short value;
{
    if (fastmode)
    {
  	send8(value >> 8);
  	send8(value);
    } else
    {
        send6(value);
        send6(value >> 6);
        send6(value >> 12);
    }
    fullcheck();
}


       /*
	*  sendss - send an array of Shorts
	*
	*/
sendss(values, nvals)
register short values[];
long nvals;
{
    register long i, nlongs;
    long along; 
    register short *ptr = (short *)&along;
    register char *cptr = (char *)values;

    nlongs = (nvals + 1) >> 1;
    sendl(nvals << 1);
    if (fastmode)
    {
	for (i=0; i<nlongs; i++)
	{
	    if (dosync(i) )
		putgchar(AESC);
	    send8( cptr[swdelem0] );
	    send8( cptr[swdelem1] );
	    send8( cptr[swdelem0+2] );
	    send8( cptr[swdelem1+2] );
	    fullcheck();
	    cptr += 4;
	}
    } else
    {
	for (i=0; i<nlongs; i++)
	{
	    if (dosync(i) )
		putgchar(AESC);
	    ptr[swdelem0] = *values++;
	    ptr[swdelem1] = *values++;
	    sendl(along);
	}
    }
    arrayflush();
}


       /*
	*  sendl - send a Long
	*
	*/
sendl(value) 
unsigned long value;
{
    register unsigned long val;
    register unsigned char *ptr = (unsigned char *)&value;

    if (fastmode)
    {
/*
 *	send8( ptr[lwdelem0] );
 *	send8( ptr[lwdelem1] );
 *	send8( ptr[lwdelem2] );
 *	send8( ptr[lwdelem3] );
 */
	val = value;
	send8(val >> 24);
	send8(val >> 16);
	send8(val >> 8);
	send8(val);
    } else
    {
	val = value;
        send6(val);
        send6(val >> 6);
        send6(val >> 12);
        send6(val >> 18);
        send6(val >> 24);
        send6(val >> 30);
    }
    fullcheck();
}


       /*
	*  sendls - send an array of Longs
	*
	*/
sendls(values, nvals)
register long values[];
register long nvals;
{
    register long i;

    sendl(nvals << 2);
    for (i=0; i<nvals; i++)
    {
	if (dosync(i) )
            putgchar(AESC);
	sendl(*values++);
    }
    arrayflush();
}


       /*
	*  sendf - send a Float
	*
	*/
sendf(value) 
float value;
{
    sendl( F2IEEE(value) );
}


       /*
	*  sendfs - send an array of Floats
	*
	*/
sendfs(values, nvals)
register float values[];
register long nvals;
{
    register long i;

    sendl(nvals << 2);
    for (i=0; i<nvals; i++)
    {
	if (dosync(i) )
            putgchar(AESC);
	sendl( F2IEEE(*values++) );
    }
    arrayflush();
}


       /*
	*  recO - receive a bOolean
	*
	*/
unsigned long recO()
{
    return recL();
}


       /*
	*  recOs - receive an array of bOoleans
	*
	*	This function is not used by lib.c, and
	*	does not work properly (GB).
	*	It has therefore been commented out pending possible need.
	*		RDG, Jan 1985
	*
recOs(values)
char *values;
{
    recLs(values);
}
	*	End of commented out function.
	*/


       /*
	*  recB - receive a Byte
	*
	*/
unsigned long recB()
{
    register unsigned char val;

    while (getgchar() != RESC) 
	;
    val = rec6();
    val |= rec6() << 6;
    return val;
}


       /*
	*  recBs - receive an array of Bytes
	*
	*  GB - values must be an array!!
	*/
recBs(values)
register char values[];
{
    register long i, nlongs;
    register unsigned long val;
    long nbytes;
    unsigned long along;
    register char *ptr = (char *)&along;

    nlongs = ((nbytes = recL()) + 3) >> 2;
    if (getgchar() != RESC)
    {
	printf("recBs: error in array transport\n");
	return;
    }
    for (i=0; i<nlongs; i++)
    {
        val = rec6();
        val |= rec6() << 6;
        val |= rec6() << 12;
        val |= rec6() << 18;
        val |= rec6() << 24;
        val |= rec6() << 30;
	if (dosync(i) )
	{
            getgchar();
	    putgchar(AESC);
	    flushg();
	}
	along = val;
	if (i < nlongs-1)
	{
	    *values++ = ptr[lwdelem0];
	    *values++ = ptr[lwdelem1];
	    *values++ = ptr[lwdelem2];
	    *values++ = ptr[lwdelem3];
	} else
	{
	    switch (nbytes & 3)
	    {
		case 0:
		    values[3] = ptr[lwdelem3];
		case 3:
		    values[2] = ptr[lwdelem2];
		case 2:
		    values[1] = ptr[lwdelem1];
		case 1:
		    values[0] = ptr[lwdelem0];
		    break;
	    }
	}
    }
    getgchar();
}


       /*
	*  recF - receive a Float
	*
	*/
float recF()
{
    return IEEE2F(recL());
}


       /*
	*  recFs - receive an array of Floats
	*
	*/
recFs(values)
register float *values;
{
    register long i, nlongs;
    register unsigned long val;

    nlongs = recL();
    if (getgchar() != RESC)
    {
	printf("recFs: error in array transport\n");
	return;
    }
    for (i=0; i<nlongs; i++)
    {
        val = rec6();
        val |= rec6() << 6;
        val |= rec6() << 12;
        val |= rec6() << 18;
        val |= rec6() << 24;
        val |= rec6() << 30;
	if (dosync(i) )
	{
            getgchar();
	    putgchar(AESC);
	    flushg();
	}
        *values++ = IEEE2F(val);
    }
    getgchar();
}


       /*
	*  recL - receive a Long
	*
	*/
unsigned long recL()
{
    register unsigned long val;

    while (getgchar() != RESC) 
	;
    val = rec6();
    val |= rec6() << 6;
    val |= rec6() << 12;
    val |= rec6() << 18;
    val |= rec6() << 24;
    val |= rec6() << 30;
    return val;
}


       /*
	*  recLs - receive an array of Longs
	*
	*/
recLs(values)
register unsigned long *values;
{
    register long i, nlongs;
    register unsigned long val;

    nlongs = recL();
    if (getgchar() != RESC)
    {
	printf("recLs: error in array transport\n");
	return;
    }
    for (i=0; i<nlongs; i++)
    {
        val = rec6();
        val |= rec6() << 6;
        val |= rec6() << 12;
        val |= rec6() << 18;
        val |= rec6() << 24;
        val |= rec6() << 30;
	if (dosync(i) )
	{
	    getgchar();
	    putgchar(AESC);
	    flushg();
	}
    	*values++ = val;
    }
    getgchar();
}


       /*
	*  recS - receive a Short
	*
	*/
unsigned long recS()
{
    register unsigned short val;

    while (getgchar() != RESC) 
	;
    val = rec6();
    val |= rec6() << 6;
    val |= rec6() << 12;
    return val;
}


       /*
	*  recSs - receive an array of Shorts
	*
	*/
recSs(values)
register short *values;
{
    register long i, nlongs;
    register unsigned long val;
    long nshorts;
    unsigned long along;
    register short *ptr = (short *)&along;

    nlongs = ((nshorts = recL()) + 1) >> 1;
    if (getgchar() != RESC)
    {
	printf("recSs: error in array transport\n");
	return;
    }
    for (i=0; i<nlongs; i++)
    {
	val =  rec6();
	val |= rec6() << 6;
	val |= rec6() << 12;
	val |= rec6() << 18;
	val |= rec6() << 24;
	val |= rec6() << 30;
	if (dosync(i) )
	{
	    getgchar();
	    putgchar(AESC);
	    flushg();
	}
	along = val;
	if (i < nlongs - 1)
	{
	    *values++ = ptr[swdelem0];
	    *values++ = ptr[swdelem1];
	} else
	    switch (nshorts & 1)
	    {
		case 0:
		    values[1] = ptr[swdelem1];
		case 1:
		    values[0] = ptr[swdelem0];
		    break;
	    }
    }
    getgchar();
}


       /********************************************
	*                                          *
	*  lib.c calls requiring no data transfer  *
	*                                          *
	********************************************/


       /*
	*  flushg - flush the graphics stream
	*
	*/
flushg()
{
    if (wp != outbuff)
    {
	fflush(stdout);
#ifndef TCP_ONLY
	if (haveether)
	    netwrite(irisfile, outbuff, wp - outbuff);
	else
#endif
	    write(irisfile, outbuff, wp - outbuff);
        wp = outbuff;
    }
}


       /*
	*  echoff - Turn local echoing off
	*
	*/
echoff()
{
    if (!netinited)
	netinit();
#ifdef UNIX4_2
    ttydat.sg_flags &= ~ECHO;
    stty(irisfile, &ttydat);
#endif
#ifdef SYSTEM5
    ttydat.c_lflag &= ~ECHO;
    ioctl(irisfile, TCSETA, &ttydat);
#endif
}


       /*
	*  echoon - Turn local echoing on
	*
	*/
echoon()
{
#ifdef UNIX4_2
    ttydat.sg_flags |= ECHO;
    stty(irisfile, &ttydat);
#endif
#ifdef SYSTEM5
    ttydat.c_lflag |= ECHO;
    ioctl(irisfile, TCSETA, &ttydat);
#endif
}


reccr()
{
    getgchar();
}


endprim()
{
}


Boolean
setfastcom()
{
    if (!netinited)
        netinit();
    if (xsetfastcom()) {
	fastmode = 1;
	return (TRUE);
    }
    else
	return (FALSE);
}


Boolean
setslowcom()
{
    if (!netinited)
        netinit();
    if (xsetslowcom()) {
	fastmode = 0;
	return (TRUE);
    }
    else
	return (FALSE);
}


       /*
	*  ginit - invokes xginit which takes special action on the terminal
	*
	*/
void
ginit()
{
    if (!netinited)
	netinit();
    xginit();
    flushg();
}


       /*
	*  greset - invokes xgreset which takes special action on the terminal
	*
	*/
void
greset()
{
    if (!netinited)
	netinit();
    xgreset();
    flushg();
}


void
gflush()
{
    if (!netinited)
	netinit();
    flushg();
}


void
gexit()
{
    if (!netinited)
	netinit();
    xgexit();
    flushg();
    ttyrestore();
}


void
gfinish()
{
    if (!netinited)
	netinit();
    flushg();
}
