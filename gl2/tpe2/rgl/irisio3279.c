/*
**		   	Encode and decode byte stream data
**
**			    Paul Haeberli - Aug 1983
*/
#include "rpc.h"
#include "term.h"
#include "Venviron.h"
#include "Vioprotocl.h"
#include "hostio.h"

short 	fastmode = 0;  /* if 1 then in fast lane */
short   turnaround = 0;
long 	a_retval;
long 	a_rvals[20];
char 	*a_arrayaddr[20];
short 	arraycount;
char 	localbuffer[200];
short 	localbufferavailable = 1;

#define rec8()		gethostchar()
#define send6(n)	puthostchar( ((n)&077) + ' ' )
#define dosync(i)	( ((i)&7) == 0 )

#ifdef IBM
unsigned char bytetable[] =
    {
    0,0,0,1,0,0,0,0,	/* ^@ - ^G */
    0,0,1,0,0,1,0,0,	/* ^H - ^O */
    0,0,1,0,0,0,0,0,	/* ^P - ^W */
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
    0,0,0,1,0,0,0,0,	/* ^@ - ^G */
    0,0,1,0,0,1,0,0,	/* ^H - ^O */
    0,0,1,0,0,0,0,0,	/* ^P - ^W */
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
    register int onechar; 

    while( bytetable[onechar=gethostchar()] == 0) 
	;
    return (onechar & 0x7f) - ' ';
}
#else
#define rec6()		( (gethostchar() &0x7f) - ' ' )
#endif

/*
**	receivef - uses the format string f to translate the incomming
**		   character stream into bytes, int and floats in a stack
** 		   frame. This function returns the number of longs in the
**		   stack frame.	
*/
receivef(f, arg) 
register char *f; register long *arg;
{	
    register short retcount;
    
    retcount = arraycount = 0;

    ++f; 	/* skip the first format item - it is a return value */

    while(*f) {
	if(*f == 'f' || *f == 'l') {
	    f++;
	    frecl(arg++);
	} else {
	    switch (*f++) {
		case 'b':
		case 'o':
			*arg++ = recb();
			break;
		case 's':
			*arg++ = recs(); 
			break;
		case 'c':
		case 'a':
			if( (*arg++ = (long)reca()) == NULL )
			    return -1;
			break;
		case 'B':
		case 'O':
		case 'S':
		case 'L':
		case 'F':
			*arg++ = (long)(a_rvals+(retcount++));
			break;	
		default:
			message("iris: bad format char\n"); 
			break;
	    }
	}
    }
    return 1;
}

/*
**	allocarray - allocate an array of bytes
**
*/
char *alloca( nbytes )
int nbytes;
{
    register char *ptr;

    if (nbytes < 200 && localbufferavailable) {
	ptr = localbuffer;
	localbufferavailable = 0;
    } else {
	ptr = (char *)malloc( nbytes );
	if(ptr != NULL)
	    a_arrayaddr[arraycount++] = ptr;
	else
	    message("alloca: can't allocate terminal buffer\n");
    }
    return ptr;
}

/*
**	freearrays - free any arrays malloced by allocarray
**
*/
freearrays()
{
    register short i;

    localbufferavailable = 1;
    for(i=0; i<arraycount; i++)
	free(a_arrayaddr[i]);
    arraycount = 0;
}

/*
**	reca - receive an array
**
*/
char *reca()
{
    register long i, nlongs;
    register long *aptr, *lptr;

    nlongs = ((recl()+3)>>2);
    if( (aptr = allocLa(nlongs)) == NULL) {
        message("iris: not enough memory for array\n");
        return NULL;
    } else {
        lptr = aptr;
        for(i=0; i<nlongs; i++) {
	    if( dosync(i) )
		if( (gethostchar() & 0x7f) != AESC) {
	            message("iris: error in array transport\n\r");
	            return NULL;
	    }
	    frecl(lptr++);
        }
	return (char *)aptr;
    }
}

/*
**	senda - send an array to the host
**
*/
senda( buffer, nlongs )
register long *buffer;
register int nlongs;
{
    register long i;
    register unsigned long val;

    puthostchar(RESC);
    for(i = 0; i<nlongs; i++) {
   	val = *buffer++;
        send6(val);
        send6(val>>=6);
        send6(val>>=6);
        send6(val>>=6);
        send6(val>>=6);
        send6(val>>6);
/*	if(dosync(i)) {
            puthostchar('\r');
            flushhost();
	    if( (gethostchar() & 0x7f) != AESC) {
	        message("iris: error in array transport senda\n");
	        return;
	    }
        }*/
    }
    puthostchar('\r');
    flushhost();
}

sendLs(array, n)
long *array, n;
{
    sendL(n);
    senda(array,n);
}

sendFs(array, n)
long *array, n;
{
    sendL(n);
    senda(array,n);
}

sendSs(array, n)
long *array, n;
{
    sendL(n);
    senda(array,(n+1)>>1);
}

sendBs(array, n)
long *array, n;
{
    sendL(n);
    senda(array,(n+3)>>2);
}


/*
**	recgcmd - receive a graphics command
**
*/
recgcmd()
{
    register cmd;

    cmd = rec6();
    cmd |= rec6()<<6;
    return cmd;
}

/*
**	recb - receive a byte value
**
*/
recb()
{
    register short byte;

    if(fastmode)
        byte = rec8();
    else {
        byte = rec6();
        byte |= rec6()<<6;
    }
    printf("B%02x-",byte);
    return byte & 0xff;
}

/*
**	recs - receive a short value
**
*/
recs()
{
    short sval;
    register char *ptr; 

    if(fastmode) {
	 ptr = (char *)&sval;    
	 *ptr++ = rec8();
	 *ptr = rec8();
    } else {
        sval = rec6();
        sval |= rec6()<<6;
        sval |= rec6()<<12;
    }
    printf("S%04x-",sval);
    return sval;
}

/*
**	recl - receive a long value
**
*/
long recl()
{
    long lval;
    register unsigned char *ptr, *inptr;

    if(fastmode) {
	ptr = (unsigned char *)&lval;    
	*ptr++ = rec8();
	*ptr++ = rec8();
	*ptr++ = rec8();
	*ptr = rec8();
    } else {
        lval = rec6();
        lval |= rec6()<<6;
        lval |= rec6()<<12;
        lval |= rec6()<<18;
        lval |= rec6()<<24;
        lval |= rec6()<<30;
    }
    printf("L%x-",lval);
    return lval;
}

/*
**	frecl - faster receive long.  assumes rec8 does the following:
**		#define gethostchar() 	
**				(--rc >= 0 ? *rp++ : fillhostbuffer())
**
*/
frecl( ptr )
register unsigned char *ptr;
{
    register unsigned char *inptr;

    if(fastmode && rc>=4) {
	inptr = rp;
	*ptr++ = *inptr++;
	*ptr++ = *inptr++;
	*ptr++ = *inptr++;
	*ptr = *inptr++;
	rp = inptr;
	rc -= 4;
    } else 
	*(long *)ptr = recl();
}

/*
**	senddata - uses the format string f to determine what values are
**		   expected by the host.
*/
senddata(f) 
register char *f;
{	
	register short retcount;

	turndelay();
	switch (*f++) {
	    case 'B':
	    case 'O':
		    sendB((char)a_retval);
		    break;
	    case 'S':
		    sendS((short)a_retval);
		    break;
	    case 'L':
		    sendL((long)a_retval);
		    break;
	    case 'F':
		    sendL((long)a_retval);
		    break;
	}

	retcount = 0;
	while(*f) {
		switch (*f++) {
		    case 'O':
		    case 'B':
			    sendB(*(char *)(a_rvals + retcount++));
			    break;
		    case 'S':
			    sendS(*(short *)(a_rvals + retcount++));
			    break;
		    case 'L':
			    sendL(*(long *)(a_rvals + retcount++));
			    break;
		    case 'F':
			    sendL(*(long *)(a_rvals + retcount++));
			    break;
		}
 	}
	puthostchar('\r');
	flushhost();
}

/*
**	sendB - send a byte value
**
*/
sendB( val )
register unsigned char val;
{
    printf("sendB %x ",val);
    puthostchar(RESC);
    send6(val);
    send6(val>>6);
}

/*
**	sendS - send a short value
**
*/
sendS( val )
register unsigned short val;
{
    printf("sendS %x ",val);
    puthostchar(RESC);
    send6(val);
    send6(val>>=6);
    send6(val>>6);
}

/*
**	sendL - send a long value
**
*/
sendL( val )
register unsigned long val;
{
    printf("sendL %x ",val);
    puthostchar(RESC);
    send6(val);
    send6(val>>=6);
    send6(val>>=6);
    send6(val>>=6);
    send6(val>>=6);
    send6(val>>6);
}

turndelay()
{
    if(turnaround)
	Delay(0,turnaround);
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
    return 100;
}

tadelay( d )
short d;
{
    if(d<0) d = 0;
    if(d>1000) d = 1000;
    turnaround = d/10;
}

ignore()
{
}
