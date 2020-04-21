/*
**		   A standalone XNS sequenced packet protocol
**
**			    Paul Haeberli - July 1983 
**
**
**	Notice that this is a limited implementation:
**
**	1. Only one connection may be open at a time.
**	2. A connection may only be opened in requestor mode.
*/

#include "net.h"
#include "Qdevices.h" 
#include "Qglobals.h" 

# undef DEBUG do_debug
# include "dprintf.h"


#define SYSPKTS		16 	/* no of system packets to be sent */
				/* without ack before closing conn */


#define SYSPKTCLICKS    2000    /* no of 1 ms clicks twixt system packets */ 
#define WINDOWSIZE	16	/* max number of outstanding packets */

PktBuf 			databuf;	/* data packet	*/
PktBuf 			systembuf;	/* system packet */
int  			SeqConnOpen;	/* true if connection open */
Xaddr			SeqAddr;	/* Destination addrs of pkts  */
unsigned short  	SeqLocId;	/* local connection id */
unsigned short  	SeqRemId;	/* remote connection id */
unsigned short  	SeqTxSeqNo;	/* transmit seq no */
unsigned short  	SeqRxAckNo;	/* receive ack */
unsigned short  	SeqTxAckNo;	/* transmit ack */
unsigned short  	SeqRxAllocNo;	/* receive alloc number */
unsigned short  	SeqTxAllocNo;	/* transmit alloc number */

unsigned 		IdLocSocket;	/* the local idp socket number */

Xnet 			MyNetwork;
extern Xhost 		MyHostAddress;
char *seqdataptr;
extern char *readdatabuf;

short piggy;

/*
**	InitSeq - allocate memory for the two packet buffers
**		  this should only be called once!
**
*/
short DidInitSeq;
InitSeq()
{
    extern short DidInitEnet;

    if( !DidInitEnet )
	if( !InitEnet() )
	    return FALSE;

    if( DidInitSeq )
	return TRUE;

    SeqConnOpen = FALSE;
    databuf = (PktBuf)gmalloc(sizeof(struct pbuf));
    systembuf = (PktBuf)gmalloc(sizeof(struct pbuf));
    if( databuf == 0 || systembuf == 0 )
	return FALSE;
    else
	return DidInitSeq = TRUE;
}

/*
**	OpenSeq - open a sequenced packet socket for reading and writting
**	  	  in REQUESTOR mode only.
**
*/
int
OpenSeq( localsocket, dstaddr )
int localsocket;
Xaddr dstaddr;
{
    register Xseqheader *seqhead;
    register Xidheader *idhead;
    register PktBuf packet;
    register int clicks, syspkts;

    if( SeqConnOpen )
	return FALSE;

    MyNetwork.high = 0x00;
    MyNetwork.low = 0x00;
    SeqAddr = dstaddr;
    if( dstaddr.socket == ANYSOCKET )
	return FALSE;
    SeqLocId = NewSeqConnId();
    SeqRemId = UNKNOWN_CONN_ID;
    SeqTxSeqNo = 0;
    SeqRxAckNo = 0;
    SeqTxAckNo = 0;
    SeqRxAllocNo = SeqRxAckNo+WINDOWSIZE;
    SeqTxAllocNo = 1;
    IdLocSocket = localsocket; 
    EnetReadTimeout(SYSPKTCLICKS);
    packet = databuf;
    syspkts = SYSPKTS;
    while(1) {
	do {
	    SendSystemPacket(SENDACK,DST_SYSTEM);
	    if ( --syspkts == 0 )
	        return FALSE;
	    packet->dataptr = readdatabuf;
        } while( (packet->length=ReadEnet(IDETHERTYPE)) == READTIMEOUT);
	packet->dataptr += sizeof( etheader );
   	idhead = (Xidheader *)packet->dataptr;
	packet->dataptr += sizeof( Xidheader );
	seqhead = (Xseqheader *)packet->dataptr;
	if( (idhead->dst.socket==IdLocSocket) && (idhead->idtype==SEQTYPE) &&
		(seqhead->dstid==SeqLocId) && (seqhead->seqno==0) &&
						(seqhead->ackno==0)  )	{
	    if( seqhead->dtype == DST_END || seqhead->dtype == DST_OEND )
		return FALSE;
	    SeqAddr = idhead->src;
	    SeqRemId = seqhead->srcid;
	    SeqConnOpen = TRUE;
	    return TRUE;
        }
    }
}

/*
**	CloseSeq - close a seq connection
**
*/
CloseSeq()
{
    if(SeqConnOpen) {
	SendSystemPacket(0,DST_END);
	SeqConnOpen = FALSE;
    }
    return TRUE;
}

/*
**	ReadSeq -  read from a sequenced packet connection
**
*/
ReadSeq()
{
extern short urgkeyin;
    register Xseqheader *seqhead;
    register Xidheader *idhead;
    register PktBuf packet;
    register int clicks, syspkts;
    register int recbytes;

dprintf(("[enter ReadSeq] "));
    if ( !SeqConnOpen ) {
dprintf(("[not open!] "));
	return -1;
    }
    EnetReadTimeout(SYSPKTCLICKS);
    packet = databuf;
    syspkts = SYSPKTS;
    while(1) {
	packet->dataptr = readdatabuf;
	if(piggy) {
	    piggy=0;
	    dprintf((" piggy"));
	} else
    	    while( (packet->length=ReadEnet(IDETHERTYPE)) == READTIMEOUT ) {
		if(urgkeyin)
		    if(ready(KEYIN))
			return READTIMEOUT;
dprintf(("[ether read timeout] "));
	        SendSystemPacket(SENDACK,DST_SYSTEM);
	        if( --syspkts == 0 ) {
		    if(urgkeyin)
			return READTIMEOUT;
dprintf(("[seq read timeout] "));
		    return -1;
		}
	    }
	packet->dataptr += sizeof( etheader );
   	idhead = (Xidheader *)packet->dataptr;
	packet->length = idhead->length;
  	seqhead = (Xseqheader *)(packet->dataptr + sizeof(Xidheader));
dprintf((" LEN:%d ",packet->length));
	if( (idhead->dst.socket==IdLocSocket) && (idhead->idtype==SEQTYPE) &&
	     					(seqhead->dstid==SeqLocId) ) {
dprintf((" dtype%d",seqhead->dtype));
     	    if( seqhead->dtype == DST_END || seqhead->dtype == DST_OEND ) {
dprintf(("[DSTEND] "));
		dprintf((" END"));
	        return -1;
	    } else if( ReadDoneSeq(packet) && FinishReadSeq(packet) ) {
	        seqdataptr = packet->dataptr;
dprintf(("[got seq data] "));
	        return packet->length;
	    }
        } else {
	    dprintf(("[GOODEST: %d %d %d ]",IdLocSocket,SEQTYPE,SeqLocId));
	    dprintf(("[BADDEST: %d %d %d ]",idhead->dst.socket,idhead->idtype,seqhead->dstid));
	}
    }
}

/*
**	ReadDoneSeq - a packet has just arrived.
**
*/
ReadDoneSeq( packet )
register PktBuf	packet;
{
    register Xseqheader *seqhead;
    register Xidheader *idhead;
    register int MustSendAck;
    
    idhead = (Xidheader *) packet->dataptr;
    packet->dataptr += sizeof( Xidheader );
    packet->length -= sizeof( Xidheader );
    seqhead = (Xseqheader *) packet->dataptr;
    if( idhead->length < (sizeof(Xidheader)+sizeof(Xseqheader)) )
	return FALSE;
    if( idhead->length > (sizeof(Xidheader)+sizeof(Xseqheader)+1024) )
	return FALSE;
    if( !HostSame(&idhead->src.host,&SeqAddr.host) )
        return FALSE;
    MustSendAck = seqhead->control & SENDACK;
    SeqTxAllocNo = seqhead->allocno;
    if(SeqTxAckNo != seqhead->ackno) {
	if(seqhead->ackno > SeqTxSeqNo) 
        	return FALSE;
        SeqTxAckNo = seqhead->ackno;
    }
    if( (seqhead->seqno == SeqRxAckNo) && !(seqhead->control & SYSTEMPACKET) ) {
        SeqRxAckNo = seqhead->seqno + 1;
        if( MustSendAck )
	    SendSystemPacket(0,DST_SYSTEM); 
        return TRUE;
    }
    if( MustSendAck )
        SendSystemPacket(0,DST_SYSTEM); 
    return FALSE;
}

/*
**	FinishReadSeq - take the sequenced packet header off the packet
**		       	and return FALSE if the data stream type is not
**		 	DST_DATA.
**
*/
FinishReadSeq( packet )
register PktBuf packet;
{
    register Xseqheader *seqhead;

    SeqRxAllocNo = SeqRxAckNo+WINDOWSIZE;
    seqhead =  (Xseqheader *)packet->dataptr; 
    packet->dataptr += sizeof(Xseqheader);
    packet->length  -= sizeof(Xseqheader);
    return (seqhead->dtype == DST_DATA || seqhead->dtype == DST_OLDDATA);
}


/*
**	WriteSeq - write to a sequenced packet connection
**
*/
int
WriteSeq( buffer, nbytes )
    char *buffer;
    int nbytes;
{
    return TypedWriteSeq(DST_DATA,buffer,nbytes);
}
TypedWriteSeq( dtype, buffer, nbytes )
char buffer[];
int nbytes;
{
    register Xseqheader *seqhead;
    register Xidheader *idhead;
    register PktBuf packet;
    register int syspkts, count;
    register int retry;

    if( ! SeqConnOpen )
	return -1;
    EnetReadTimeout(SYSPKTCLICKS);
    packet = databuf;
    retry = SYSPKTS;
    while(1) {
        do {
 	    EmptyPkt(packet);
	    count = MIN(1024,nbytes); 
	    blt(packet->dataptr,buffer,count);
	    packet->length = count;
            packet->dataptr -= sizeof(Xseqheader);
            packet->length  += sizeof(Xseqheader);
            seqhead = (Xseqheader *)packet->dataptr;
            seqhead->control = SENDACK;
            seqhead->dtype = dtype;
            seqhead->srcid = SeqLocId;
            seqhead->dstid = SeqRemId;
            seqhead->seqno = SeqTxSeqNo;
            seqhead->ackno = SeqRxAckNo;
            seqhead->allocno = SeqRxAllocNo;
            WriteId(packet);
            if ( --retry == 0 ) {
	        return -1;
	    }
	    packet->dataptr = readdatabuf;
        } while( (packet->length=ReadEnet(IDETHERTYPE)) == READTIMEOUT );
	packet->dataptr += sizeof( etheader );
   	idhead = (Xidheader *)packet->dataptr;
	packet->length = idhead->length;
	packet->dataptr += sizeof( Xidheader );
	packet->length -= sizeof( Xidheader );
	seqhead = (Xseqheader *)packet->dataptr;
	packet->dataptr += sizeof( Xseqheader );
	packet->length -= sizeof( Xseqheader );
	if( (idhead->dst.socket==IdLocSocket) && (idhead->idtype==SEQTYPE) &&
	     (seqhead->dstid==SeqLocId) ) {
	    if( seqhead->dtype == DST_END || seqhead->dtype == DST_OEND )
	        return FALSE;
	    if( seqhead->ackno== (SeqTxSeqNo+1) ) {
if(!(seqhead->control&SYSTEMPACKET)) piggy=1;
	        SeqAddr = idhead->src;
	        SeqRemId = seqhead->srcid;
                SeqTxSeqNo++;
	        return count;
	    }
        }
    }
}


/*
**	SendSystemPacket - send a system packet
**
*/
SendSystemPacket( Control, DataStype )
register int Control;
int DataStype;
{
    register Xseqheader *seqhead;
    register PktBuf packet;
 
    packet=systembuf;
    EmptyPkt(packet);
    packet->dataptr -= sizeof(Xseqheader);
    packet->length  += sizeof(Xseqheader);
    seqhead = (Xseqheader *)packet->dataptr;
    seqhead->control = SYSTEMPACKET | Control;
    seqhead->dtype = DataStype;
    seqhead->srcid = SeqLocId;
    seqhead->dstid = SeqRemId;
    seqhead->seqno = SeqTxSeqNo;
    seqhead->ackno = SeqRxAckNo;
    seqhead->allocno = SeqRxAllocNo;
    WriteId(packet);
}

/*
**	WriteId - add the id and ether headers to the  packet and write
**		  it out now
**
*/
WriteId( packet )
register PktBuf packet;
{
    register Xidheader *idhead;
    register etheader *ethead;

    packet->dataptr -= sizeof(Xidheader);
    packet->length  += sizeof(Xidheader);
    idhead = (Xidheader *)packet->dataptr;
    idhead->checksum = NOCHECKSUM;
    idhead->length = packet->length;
    idhead->control = 0;
    idhead->idtype = SEQTYPE;
    idhead->src.net = MyNetwork;
    idhead->src.host = MyHostAddress;
    idhead->src.socket = IdLocSocket;
    idhead->dst.net = MyNetwork;
    idhead->dst.host = SeqAddr.host;
    idhead->dst.socket = SeqAddr.socket;
    packet->dataptr -= sizeof(etheader);
    packet->length  += sizeof(etheader);
    ethead = (etheader *)packet->dataptr;
    ethead->dst = SeqAddr.host;
    WriteEnet(IDETHERTYPE,packet->dataptr,packet->length);
}

/*
**	NewSeqConnId - make a new sequenced packet protocol connection
**		       id from the clock.  This should never return 0
**		       because zip means connection id unknown.
**
*/
NewSeqConnId()
{
    if( (Qupcount & 0xffff) == 0)
	return 1;	
    else
        return Qupcount & 0xffff;
}

/*
**	HostSame - return TRUE if host addresses are identical
**
*/
HostSame( Host1, Host2 )
register Xhost *Host1, *Host2;
{
    return( (Host1->low  == Host2->low ) &&
            (Host1->mid  == Host2->mid ) &&
            (Host1->high == Host2->high) );
}

/*
**	EmptyPkt - prepare a packet to be filled from the middle down and up
**
*/
int EmptyPkt( packet )
register PktBuf packet;
{
    packet->size = PKTBUFSIZE;
    packet->length = 0;
    packet->dataptr = packet->data+PKTBUFSIZE-MAXDATA;
}
