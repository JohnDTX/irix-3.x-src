#include "duart.h"

/*
	At init:
		set OPCR, ACR, IMR
 */

duart *dad[] = {
	(duart *)(D0A + 0*DINCR),
	(duart *)(D0A + 1*DINCR),
	(duart *)(D1A + 0*DINCR),
	(duart *)(D1A + 1*DINCR)
};

long aa[5];

int argc; char **argv;

main()
{

    for( ;; )
    {
    printf("args (duart speed count):");
    readargs(&argc,&argv);
    aa[0] = 2;
    aa[1] = 19200;
    aa[2] = -1;
    numargs(argc,argv,aa,0,3);
    msdelay(2000);
    printf("setbaud(%d,%d)\n",aa[0],aa[1]);
    setbaud(aa[0],aa[1]);
    msdelay(1000);
    printf("jam(%d,%d)\n",aa[0],aa[2]);
    msdelay(2000);
    jam(aa[0],aa[2]);
    }
}


char *beginstring = 
"\020  \
\0203!\
\020Y\"\
\020B    \
\0208 ";
char *string =
"\020B!            \
\020B!!  _#       \
\020B!\"  _# _#    \
\020B!#  0\" 0\" 0\" \
\020B!$     @#    \
\020B!%  0\" _# 0\" \
\020B!&     _# _# \
\020B!'     0# 0# \
\020B!(  @\" @\" _# \
\020B!)  0#    0\" \
\020B!*  _#    _# \
\020B!+  0\"    P# \
\020B!,        _# \
\020B!-  _# P\"    \
\020B!.  _# _# _# \
\020B!/  0\" 0\" 0\" \
\020B!0           \
\020B!1  _#       \
\020B!2  _# _#    \
\020B!3  0\" 0\" 0\" \
\020B!4     @#    \
\020B!5  0\" _# 0\" \
\020B!6     _# _# \
\020B!7     0# 0# \
\020B!8  @\" @\" _# \
\020B!9  0#    0\" ";
char *endstring =
"\020X\"\
\020\\\"";


jam(duart,n)
    int duart;
    int n;
{
    register char *cp;
    register int i;

	if( *cp == 000 ) {
	    i = 50;
	    while (i-- > 0) putcraw('\0',duart);	
	    cp = string; 
	}
    for (cp = beginstring; *cp; cp++)
	putcraw(*cp, duart);
    cp = string;
    i = 0;
    while(--n > 0 || *cp)
    {
	/* throw out some lines every now and then */
	if (*cp == '\020' && (++i & 0xf) == 0) putcraw('\n',duart);
	putcraw(*cp++,duart);
    }
    for (cp = endstring; *cp; cp++)
	putcraw(*cp, duart);
    cp = "";
}


char du_speedbits[] =
{
    BAUD300,BAUD600,BAUD1200,BAUD2400,BAUD4800,BAUD9600,BAUD19200,BAUD300
};
short du_speeds[] =
{
        300,    600,    1200,    2400,    4800,    9600,    19200,0
};

setbaud(port,rate)
    int port;
    int rate;
{
    register int speedno;

    for( speedno = 0; du_speeds[speedno] != 0; speedno++ )
	if( du_speeds[speedno] == rate )
	    break;
    dad[port]->d_csr = du_speedbits[speedno];
}
