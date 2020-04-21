/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/seq.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:48 $
 */

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

#include	"net.h"
#include	"errno.h"
#include	"cntrlr.h"
#include	"dprintf.h"

extern	_xnserr;

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
char			*_seqdataptr;
extern char		*_readdatabuf;
extern Xhost 		_MyHostAddress;
short			_seqflags;

short _piggy;

/*
** seqinit
**   do the needed initializations for the sequenced packet proto.
**   Returns 1 if everything suceeds, 0 otherwise.
*/
_seqinit()
{
	extern short _nxflags;

	/*
	** if the execlan board has not been initialized - do it
	*/
	if ( ! ( _nxflags & INITED ) )
		if( !_nxinit() )
		{
	dprintf(( "SEQinit: nx init failed\n" ));
		    return (0);
		}

	/*
	** if the seq protocol has been initialized, we be done
	*/
	if ( _seqflags & INITED )
		return (1);

	/* connection is not in use */
	_seqflags &= ~INUSE;

	/*
	** allocate the data and system buffers
	*/
	databuf = (PktBuf)mbmalloc( sizeof (struct pbuf) );
	systembuf = (PktBuf)mbmalloc( sizeof (struct pbuf) );
	if ( databuf == 0 || systembuf == 0 )	/* no memory	*/
	{
	dprintf(( "SEQinit: no mem\n" ));
		_xnserr = ENOMEM;
		return (0);
	}

	_seqflags |= INITED;	/* initialization completed	*/
	return (1);
}

/*
** seqopen
**   open a sequenced packet socket for reading and writting
**   in REQUESTOR mode only.
**
*/
_seqopen( localsocket, dstaddr )
int	localsocket;
Xaddr	dstaddr;
{
    register Xseqheader	*seqhead;
    register Xidheader	*idhead;
    register PktBuf	packet;
    register int	syspkts;

	/*
	** if connection is already in use - no way!
	*/
	if ( _seqflags & INUSE )
	{
	dprintf(( "SEQopen: already open\n" ));
		_xnserr = EISCONN;
		return (0);
	}

	MyNetwork.high = 0x00;
	MyNetwork.low = 0x00;
	SeqAddr = dstaddr;
	if ( dstaddr.socket == ANYSOCKET )
	{
		_xnserr = EINVAL;
		return (0);
	}
	SeqLocId = 1;
	SeqRemId = UNKNOWN_CONN_ID;
	SeqTxSeqNo = 0;
	SeqRxAckNo = 0;
	SeqTxAckNo = 0;
	SeqRxAllocNo = SeqRxAckNo + WINDOWSIZE;
	SeqTxAllocNo = 1;
	IdLocSocket = localsocket; 
	packet = databuf;
	syspkts = SYSPKTS;


	while ( 1 )
	{
	   do
	   {
		    _SendSystemPacket( SENDACK, DST_SYSTEM );
		    if ( --syspkts == 0 )
		    {
	dprintf(( "SEQopen: timeo (syspkts==0)\n" ));
			_xnserr = ETIMEDOUT;
			return (0);
		    }
		    packet->dataptr = _readdatabuf;
           } while( ( packet->length = _etherread( IDETHERTYPE ) ) == READTIMEOUT );
		packet->dataptr += sizeof( etheader );
		idhead = (Xidheader *)packet->dataptr;
		packet->dataptr += sizeof ( Xidheader );
		seqhead = (Xseqheader *)packet->dataptr;
		if ( ( idhead->dst.socket == IdLocSocket )
		       && ( idhead->idtype == SEQTYPE ) &&
		       ( seqhead->dstid == SeqLocId ) && ( seqhead->seqno == 0 )
		       && (seqhead->ackno == 0 ) )
		{
		    if ( seqhead->dtype == DST_END ||
			 seqhead->dtype == DST_OEND )
		    {
	dprintf(( "SEQopen: host closed\n" ));
			_xnserr = ETIMEDOUT;
			return (0);
		    }
		    SeqAddr = idhead->src;
		    SeqRemId = seqhead->srcid;
		    _seqflags |= INUSE;
		    return (1);
		}
	}
}

/*
** seqclose
**   close a sequenced connection
**
*/
_seqclose()
{
	if ( _seqflags & INUSE )
	{
		_SendSystemPacket( 0, DST_END );
		_seqflags &= ~INUSE;
	}
	return (1);
}

/*
** seqread
**   read from a sequenced packet connection
*/
_seqread()
{
    register Xseqheader	*seqhead;
    register Xidheader	*idhead;
    register PktBuf	packet;
    register int	syspkts;
    register int	recbytes;

	if ( ! ( _seqflags & INUSE ) )
	{
	dprintf(( "SEQread: no connection\n" ));
		_xnserr = ENOTCONN;
		return (-1);
	}
	packet = databuf;
	syspkts = SYSPKTS;


	while ( 1 )
	{
		packet->dataptr = _readdatabuf;
if(_piggy) {_piggy=0;} else
		while ( ( packet->length = _etherread( IDETHERTYPE ) ) ==
			  READTIMEOUT )
		{
		    _SendSystemPacket( SENDACK, DST_SYSTEM );
		    if ( --syspkts == 0 )
		    {
	dprintf(( "SEQread: timeo (syspkts==0)\n" ));
			_xnserr = ETIMEDOUT;
			return (-1);
		    }
		}
		packet->dataptr += sizeof ( etheader );
		idhead = (Xidheader *)packet->dataptr;
		packet->length = idhead->length;
		seqhead = (Xseqheader *)(packet->dataptr + sizeof(Xidheader));
		if ( ( idhead->dst.socket == IdLocSocket ) &&
		     ( idhead->idtype == SEQTYPE ) &&
		     ( seqhead->dstid == SeqLocId ) )
		{
			if ( seqhead->dtype == DST_END ||
			     seqhead->dtype == DST_OEND )
			{
	dprintf(( "SEQread: host closed\n" ));
				return (0);
			}
			else
			if ( _ReadDoneSeq( packet ) && _FinishReadSeq( packet ) )
			{
#ifdef NOTDEF
printf("seqread: data found\n" );
#endif
				_seqdataptr = packet->dataptr;
				return (packet->length);
			}
		}
	}
}

/*
** seqwrite
**   write to a sequenced packet connection
*/
_seqwrite( buffer, nbytes )
char	*buffer;
int 	nbytes;
{
    return (_TypedWriteSeq( DST_DATA, buffer, nbytes ) );
}

/*
** ReadDoneSeq
**   a packet has just arrived.
**
*/
_ReadDoneSeq( packet )
register PktBuf	packet;
{
	register Xseqheader	*seqhead;
	register Xidheader	*idhead;
	register int	MustSendAck;
    
	idhead = (Xidheader *)packet->dataptr;
	packet->dataptr += sizeof ( Xidheader );
	packet->length -= sizeof ( Xidheader );
	seqhead = (Xseqheader *)packet->dataptr;
	if ( idhead->length < ( sizeof ( Xidheader ) + sizeof ( Xseqheader ) ) )
		return (0);
	if ( idhead->length > ( sizeof ( Xidheader ) +
	     sizeof ( Xseqheader ) + 1024 ) )
		return (0);
	if ( ! _HostSame( &idhead->src.host, &SeqAddr.host ) )
		return (0);
	MustSendAck = seqhead->control & SENDACK;
	SeqTxAllocNo = seqhead->allocno;
	if ( SeqTxAckNo != seqhead->ackno )
	{
		if ( seqhead->ackno > SeqTxSeqNo ) 
			return (0);
		SeqTxAckNo = seqhead->ackno;
	}
	if ( ( seqhead->seqno == SeqRxAckNo) &&
	      ! ( seqhead->control & SYSTEMPACKET ) )
	{
		SeqRxAckNo = seqhead->seqno + 1;
		if ( MustSendAck )
		    _SendSystemPacket( 0, DST_SYSTEM ); 
		return (1);
	}
	if ( MustSendAck )
		_SendSystemPacket( 0, DST_SYSTEM ); 
	return (0);
}

/*
** FinishReadSeq
**   take the sequenced packet header off the packet
**   and return FALSE if the data stream type is not
**   DST_DATA.
**
*/
_FinishReadSeq( packet )
register PktBuf packet;
{
    register Xseqheader *seqhead;

    SeqRxAllocNo = SeqRxAckNo+WINDOWSIZE;
    seqhead =  (Xseqheader *)packet->dataptr; 
    packet->dataptr += sizeof ( Xseqheader );
    packet->length  -= sizeof ( Xseqheader );
    return (seqhead->dtype == DST_DATA || seqhead->dtype == DST_OLDDATA);
}

/*
** TypedWriteSeq
**   write data to a seq connection
*/
_TypedWriteSeq( dtype, buffer, nbytes )
char	buffer[];
int	nbytes;
{
	register Xseqheader	*seqhead;
	register Xidheader	*idhead;
	register PktBuf		packet;
	register int		syspkts,
				count,
    				retry;

	if ( ! ( _seqflags & INUSE ) )
		return (-1);
	packet = databuf;
	retry = SYSPKTS;
	while ( 1 )
	{
           do
	   {
		_EmptyPkt( packet );
		count = MIN( 1024, nbytes ); 
		bcopy( buffer, packet->dataptr, count );
		packet->length = count;
		packet->dataptr -= sizeof ( Xseqheader );
		packet->length  += sizeof ( Xseqheader );
		seqhead = (Xseqheader *)packet->dataptr;
		seqhead->control = SENDACK;
		seqhead->dtype = dtype;
		seqhead->srcid = SeqLocId;
		seqhead->dstid = SeqRemId;
		seqhead->seqno = SeqTxSeqNo;
		seqhead->ackno = SeqRxAckNo;
		seqhead->allocno = SeqRxAllocNo;
		_WriteId( packet );
		if ( --retry == 0 )
		{
	dprintf(( "TWSEQ: retry==0\n" ));
			_xnserr = ETIMEDOUT;
			return (-1);
		}
		packet->dataptr = _readdatabuf;
           } while ( ( packet->length = _etherread( IDETHERTYPE ) ) ==
		     READTIMEOUT );
		packet->dataptr += sizeof ( etheader );
		idhead = (Xidheader *)packet->dataptr;
		packet->length = idhead->length;
		packet->dataptr += sizeof ( Xidheader );
		packet->length -= sizeof ( Xidheader );
		seqhead = (Xseqheader *)packet->dataptr;
		packet->dataptr += sizeof ( Xseqheader );
		packet->length -= sizeof ( Xseqheader );
		if ( ( idhead->dst.socket == IdLocSocket ) &&
		     ( idhead->idtype == SEQTYPE ) &&
		     ( seqhead->dstid == SeqLocId ) )
		{
			if ( seqhead->dtype == DST_END ||
			     seqhead->dtype == DST_OEND )
			     {
	dprintf(( "TWSEQ: host closed\n" ));
				return (0);
			     }
			if ( seqhead->ackno == ( SeqTxSeqNo + 1 ) )
			{
if(!(seqhead->control&SYSTEMPACKET)) _piggy=1;
				SeqAddr = idhead->src;
				SeqRemId = seqhead->srcid;
				SeqTxSeqNo++;
				return (count);
			}
		}
	}
}

/*
** SendSystemPacket
**   send a system packet
*/
_SendSystemPacket( Control, DataStype )
register int	Control;
int		DataStype;
{
	register Xseqheader	*seqhead;
	register PktBuf		packet;
 
	packet = systembuf;
	_EmptyPkt( packet );
	packet->dataptr -= sizeof ( Xseqheader );
	packet->length  += sizeof ( Xseqheader );
	seqhead = (Xseqheader *)packet->dataptr;
	seqhead->control = SYSTEMPACKET | Control;
	seqhead->dtype = DataStype;
	seqhead->srcid = SeqLocId;
	seqhead->dstid = SeqRemId;
	seqhead->seqno = SeqTxSeqNo;
	seqhead->ackno = SeqRxAckNo;
	seqhead->allocno = SeqRxAllocNo;
	_WriteId( packet );
}

/*
** WriteId
**   add the id and ether headers to the  packet and write
**   it out now
*/
_WriteId( packet )
register PktBuf	packet;
{
	register Xidheader	*idhead;
	register etheader	*ethead;

    packet->dataptr -= sizeof ( Xidheader );
    packet->length  += sizeof ( Xidheader );
    idhead = (Xidheader *)packet->dataptr;
    idhead->checksum = NOCHECKSUM;
    idhead->length = packet->length;
    idhead->control = 0;
    idhead->idtype = SEQTYPE;
    idhead->src.net = MyNetwork;
    idhead->src.host = _MyHostAddress;
    idhead->src.socket = IdLocSocket;
    idhead->dst.net = MyNetwork;
    idhead->dst.host = SeqAddr.host;
    idhead->dst.socket = SeqAddr.socket;
    packet->dataptr -= sizeof(etheader);
    packet->length  += sizeof(etheader);
    ethead = (etheader *)packet->dataptr;
    ethead->dst = SeqAddr.host;
    _etherwrite( IDETHERTYPE, packet->dataptr, packet->length );
}

/*
** HostSame
**  return TRUE if host addresses are identical
*/
_HostSame( Host1, Host2 )
register Xhost	*Host1,
		*Host2;
{
	return ( ( Host1->low  == Host2->low ) &&
		 ( Host1->mid  == Host2->mid ) &&
		 ( Host1->high == Host2->high ) );
}

/*
** EmptyPkt
**   prepare a packet to be filled from the middle down and up
*/
_EmptyPkt( packet )
register PktBuf	packet;
{
	packet->size = PKTBUFSIZE;
	packet->length = 0;
	packet->dataptr = packet->data + PKTBUFSIZE - MAXDATA;
}
