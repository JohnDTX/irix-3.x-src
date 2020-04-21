/*
**		        Iris monitor serial encode/decode
**
**			    Paul Haeberli - July 1983
**
*/
#include "Qdevices.h"
#include "Qglobals.h"
#include "remprom.h"

# undef DEBUG do_debug
# include "dprintf.h"



int
serial_open()
{
    return 0;
}

int serial_close()
{
}

int
serial_read(_ptr,len)
    char (**_ptr);
    int len;
{
    short nbytes;
    unsigned short pktcheck,sum;

    if( !recshort(&nbytes) )
	return badfmt();

dprintf((" len%d",nbytes));
    if( !(0 < nbytes && nbytes <= len) )
    {
	printf("? Packet size %d out of range\n",nbytes);
	return -1;
    }

    if( recbytes(*_ptr,nbytes,&sum) < 0 )
	return -1;

    if( !recshort(&pktcheck) )
	return badfmt();

    if( pktcheck != sum )
    {
	printf("? Packet checksum 0x%x shouldbe 0x%x\n",sum,pktcheck);
	return -1;
    }

dprintf(("-"));
    return nbytes;
}

int
badfmt()
{
    printf("? Packet format error\n");
    return -1;
}



/*
**	recchar - receive a character from the host
**
*/
recchar()
{
    register int r;

    do {
        r = wait(KEYIN|FROMHOST);
        if(r&KEYIN) {
  	    wait(TOHOST);
	    PutToHost(GetKeyIn());	
	}
    } while(!(r&FROMHOST));
    return GetFromHost()&0x7f;
}

/*
**	sendchar - send a character to the host
**
*/
sendchar( onechar )
char onechar;
{
    wait(TOHOST);
    PutToHost(onechar);
}

/*
**	recbyte - receive a command from the host	
**
*/
recgcmd()
{
    register short cmd;

    cmd = (recchar() - ' ')&077;
    cmd |= ((recchar() - ' ')&077)<<6;
    return cmd;
}

/*
**	recshort - receive a short from the host
**
*/
recshort( val )
register short *val;
{
    register char onechar;

    if( (onechar=recchar()) < ' ' ) return 0;
    *val = (onechar - ' ')&077;
    if( (onechar=recchar()) < ' ' ) return 0;
    *val |= ((onechar - ' ')&077)<<6;
    if( (onechar=recchar()) < ' ' ) return 0;
    *val |= ((onechar - ' ')&017)<<12;
    return 1;
}

/*
**	sendshort - send a short to the host
**
*/
sendshort( value )
register short value;
{
    register long i;

    i = value & 0xffff;
    sendchar(PESC);
    sendchar((i&077) + ' ');
    sendchar(((i>>6)&077) + ' ');
    sendchar(((i>>12)&017) + ' ');
    sendchar('\r');
}

int
recbytes(buf,nbytes,_sum)
    register char *buf;
    register short nbytes;
    short (*_sum);
{
    short datashort;
    register int nshorts;
    register short checksum;

    nshorts = (nbytes+1)>>1;
    checksum = 0;

    while( --nshorts >= 0 )
    {
	if( !recshort(&datashort) ) 
	    return badfmt();
	checksum += datashort;
	*buf++ = (char)datashort;
	*buf++ = (char)(datashort>>8);
    }

    *_sum = checksum;
    return 0;
}
