# include "Qdevices.h"
# include "Qglobals.h"
# include "common.h"
# include "net.h"

# undef DEBUG do_debug
# include "dprintf.h"

pnetaddr()
{
    extern Xhost MyHostAddress;
    InitEnet();
    printf("HI MY NAME IS \"%x %x %x\".\n",
       MyHostAddress.high,MyHostAddress.mid,MyHostAddress.low);
}


# ifdef notdef
char *nstatnames[] = {
    "frames sent no err",
    "frames aborted excess collisions",
    "frames aborted late collision",
    "tdr",
    "frames rcv no err",
    "frames rcv align err",
    "frames rcv crc err",
    "frames lost"
};

pnetstat()
{
    long buf[8];
    register i;

    nxstats(buf);
    printf("Net stats:\n");
    for(i=0; i<8; i++)
	printf(" %s:%d\n", nstatnames[i], buf[i]);
}


extern Xhost MyHostAddress;
extern char *readdatabuf;
char *nettestbuf;

typedef struct {
    etheader et;
    Xidheader id;
    Xechoheader echo;
} Xechopacket;

/*
**	writetest - test the abilty of the board to write packets of
**		    various sizes.
*/
testwrite(n,m,idelay)
    int n,m,idelay;
{
    register int i;
    register Xserverreq *ServerReq;

    testbufalloc();
    ServerReq = (Xserverreq *)nettestbuf;
    for(i=0; i<MAX_ENET_PACKET; i++)
	nettestbuf[i] = i; 

	/*
	 * old testwrite
    for(i=60; i<=1024; ) {
        ServerReq->etherheader.dst.high = 0xffff;
        ServerReq->etherheader.dst.mid  = 0xffff;
        ServerReq->etherheader.dst.low =  0xffff;
	WriteEnet(SG_DIAG,ServerReq,i);
	if(i<80)
	    i++;
	else
	    i+=30;
	printf(("[%d]",i);
	alarm(500);
	if(wait(KEYIN|ALARM) == KEYIN) {
	    GetKeyIn();
	    break;
	}
    }
	 *
	 */
    ServerReq->etherheader.dst.high = 0x0800;
    ServerReq->etherheader.dst.mid  = 0x1400;
    ServerReq->etherheader.dst.low =  0xffff;	/* garbage address */
    while( --n >= 0 )
    {
	WriteEnet(SG_DIAG,ServerReq,m);
	if(ready(KEYIN)) {
	    GetKeyIn();
	    n = 0;
	}
	msdelay(idelay);
    }
    printf("\n\n");
}

echoserve()
{
    register Xechopacket *echopacket;
    register int nbytes;
    Xaddr xaddr;

    testbufalloc();
    echopacket = (Xechopacket *)nettestbuf;
    EnetReadTimeout(0);   /* wait forever for a packet on read */
    while( 1 ) {
        nbytes=ReadEnet(IDETHERTYPE);
	blt(nettestbuf,readdatabuf,nbytes);
	if( HostSame(&MyHostAddress,&(echopacket->et.dst)) 
		&& (echopacket->id.idtype == ECHOTYPE) 
			    && (echopacket->echo.operation == ECHOREQUEST) 
					    && (nbytes>=(32+14)) ) {
		xaddr = echopacket->id.src;
		echopacket->id.src = echopacket->id.dst;
		echopacket->id.dst = xaddr;
		echopacket->et.dst = echopacket->et.src;
		echopacket->echo.operation = ECHOREPLY;
		printf("[echo]");
		WriteEnet(IDETHERTYPE,echopacket,nbytes);
	}
    }
}

testbufalloc()
{
    extern char *gmalloc();

    if( !InitSeq() )
    {
	printf("? InitSeq failed\n");
	return;
    }
    if( nettestbuf != 0 )
	return;

    if( (nettestbuf = gmalloc(MAX_ENET_PACKET)) == 0 )
        return;
}
# endif notdef
