/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/nx.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:41 $
 */

#include	"errno.h"
#include	"nx.h"
#include	"Xns.h"	
#include	"mbenv.h"
#include	"cntrlr.h"
#include	"dprintf.h"

extern	_xnserr;

#define	NBBY	8

Xhost _MyHostAddress;

#define NXMAXBUZZ	300000
#define busfix(a)	((a)^01)

short	_nxflags;

/*
** nxinit
**   Return 0 if we fail to start the device for any reason
**   Return 1 if we succeed at everything
*/
_nxinit()
{
	if ( _nxflags & INITED )
		return (1);

	if ( _nxconfig() == 0 || _nxonce() == 0 )
		return (0);
	bcopy( &myaddr, &_MyHostAddress, 6 );	/* pass our host address */
	_readenable();
	_nxflags |= INITED;
	return (1);		/* Succeeded to start Enet Device */
}

_nxintr()
{
	register READ		rmp;
	register MP		p;
	register struct request	*request;

	rmp = (READ)lastrmp;
	while ( rmp->status == 0 )
	{
		request = (struct request *)( rmp->id );
		Dprintf(2, ("nxintr: got reply"));
		if ( request->reply )
			bcopy(rmp, request->reply, sizeof(struct xmit_msg));
		rmp->length = MSGLEN;
		rmp->id = 0;
		rmp->status = NX_BUSY;	/* free this msg buffer */
		p = (MP)rmp;		/* point to next receive msg buf */
		rmp = (READ)(p->next);
		request->complete = 1;
	}
	lastrmp = (MP)rmp;		/* remember where we left off */
}

/*
** nxconfig
**   See if the board is present and record result in nxpresent.
*/
_nxconfig()
{
	register i;

	/* calculate virtual address of port A and port B */
	PortA = (char *)mbiotov( PORTA );
	PortB = (char *)mbiotov( PORTB );

	/*
	** See if interface is present.
	** Reading Port A resets the board (probe(adr) reads from adr).
	*/
	if ( probe( PortA, 1 ) )
		_nxpresent = 1;
	else {
		PortA = (char *)mbiotov( busfix( 0x10 ) );
		PortB = (char *)mbiotov( busfix( 0x11 ) );
		if ( probe( PortA, 1 ) )
			_nxpresent = 1;
		else {
			dprintf(( "nxconfig: board not found\n" ));
				_nxpresent = 0;	
				_xnserr = ENODEV;
				return (0);
		}
	}

	/*
	 * Test status after board reset. (probing PortA above did the reset)
	 * If the Nx101 fails to respond to the reset, indicate non-presence
	 * (we might want this to be a third category of response instead)
	 */
	i = 800000;
	while ( ( *PortB & 1 ) == 0 )
		if ( --i <= 0 )
		{
	dprintf(( "nxconfig: board not responding\n" ));
		    _xnserr = ETIMEDOUT;
		    _nxpresent = 0;
		    return (0);
		}


#ifdef NOTDEF
printf("nx board found @ %x\n", PortA );
#endif

	return (1);
}

/*
** nxonce
**   First time code.
**   Gets multibus memory, starts timer, gives init message to the controller,
**   connects to the network, discovers our physical address, and gives a few
**   receive buffers to the controller.
*/
_nxonce()
{
	/* Allocate space for msg buffers, read buffer and write buffer */

	if ( ! _nxmalloc() )
	{
	dprintf(( "nxonce: no mem for bufs\n" ));
		_xnserr = ENOBUFS;
		return (0);
	}

	/* initialize the message queues and enable the controller */

	if ( ! _nxqsetup() )
	{
	dprintf(( "nxonce: can't init queues\n" ));
		return (0);
	}
	if ( _nxmode( CONNECT_TO_NET ) == 0 )
	{
	dprintf(( "nxonce: can't connect to net\n" ));
		return (0);
	}
	if ( _getslot( 253, &myaddr ) == 0 )	/* read our address */
	{
	dprintf(( "nxonce: can't read addr\n" ));
		return (0);
	}
	return (1);
}

struct system_buffers
{
    char		readbuffer[ MAXPACKET ],
    			writebuffer[ MAXPACKET ];
    char		readreply[ sizeof (struct xmit_msg) ];
    struct request	readrequest,
			writerequest;
};

/*
** nxmalloc
**   Allocate multibus memory for operating the dma and message queues.
**   Leave the virtual address and physical multibus address as side-effects
**   in global variables nxva and nxpa.
*/
_nxmalloc()
{
	struct system_buffers	*system_buf;
	long			offset;
               
	/* 
	** allocate space for the system buffers on the multibus,
	** then initialize the fixed global pointers into this area
	*/
	system_buf = (struct system_buffers *)
		mbmalloc( sizeof (struct system_buffers) );
	if ( system_buf == 0 )		/* if we didn't get the mem -> fail */
	{
	dprintf(( "nxmalloc: no mem for sys bufs\n" ));
		return (0);
	}

	_readdatabuf = system_buf->readbuffer;
	readreply   = system_buf->readreply;
	readrequest = &system_buf->readrequest;
	writedatabuf = system_buf->writebuffer;
	writerequest = &system_buf->writerequest;

	/* 
	** Now allocate space for the host-controller message queues
	*/
	nxva = (long)mbmalloc( sizeof (struct mqueues) + 1024 );
	if ( nxva == 0 )
	{
	dprintf(( "nxmalloc: no mem for host-exos queue\n" ));
		return (0);
	}
	offset = nxva & 511;
	nxva += 512 - offset;

	nxpa = mbvtop( nxva ) & 0xffffff;
	bzero( nxva, sizeof (struct mqueues) );
	return (1);		
}

/*
** nxqsetup
**   Set up the host/controller queues and netbuf array.
**   Construct initialization message and send it.
*/
_nxqsetup()
{
	register struct msgbuf	*p,
				*l;		/* a5, a4 */
	register struct init	*im;		/* a3 */
	register	 	i,
				x;		/* d7, d6, d5 */

	struct mqueues		*mp;
	struct testp		*testpat;
	long			addr;

	nxmp = mp = (struct mqueues *)nxva;
	mp->roffset = HTOX( mp->rbufs );
	mp->woffset = HTOX( mp->wbufs );
	lastrmp = mp->rbufs;
	mp->rhead = mp->rbufs;
	mp->whead = mp->wbufs;

	/*
	** intialize the list of exos-to-host message buffers
	*/
	l = &mp->rbufs[ NRBUFS - 1 ];
	for ( i = 0; i < NRBUFS; i++ )
	{
		p = &mp->rbufs[ i ];
		l->next = p;
		l->msg.link = HTOX( p );
		l->msg.length = MSGLEN;
		l->msg.status = 3;
		l = p;
	}

	/*
	** intialize the list of host-to-exos buffers message buffers
	*/
	l = &mp->wbufs[ NWBUFS - 1 ];
	for ( i = 0; i < NWBUFS; i++ )
	{
		p = &mp->wbufs[i];
		l->next = p;
		l->msg.link = HTOX(p);
		l->msg.length = MSGLEN;
		l->msg.status = 0;
		l = p;
	}

	/*
	** construct initialization (configuration) message.
	*/
	nxim = im = (struct init *)_readdatabuf;
	bzero( im, sizeof *im );
	im->res0 = 1;		/* indicate this is a v1.4 init message */
	im->amode = 3;
	im->naddr = 0xff;
	im->nproc = 0xff;
	im->format[ 0 ] = 1;
	im->format[ 1 ] = 1;
	testpat = (struct testp *)im->mmap;
	testpat->b0 = 1;
	testpat->b1 = 3;
	testpat->b2 = 7;
	testpat->b3 = 0xf;
	testpat->w0 = 0x103;
	testpat->w1 = 0x70f;
	testpat->l0 = 0x103070f;
	im->nmbox = 0xff;
	im->code = 0xff;
	im->hxitype = im->xhitype = 0;	/* how to interrupt */
	im->mvblk = -1L;
	im->nhost = 1;
	im->hxseg = mbvtop( nxva );
	im->hxhead = HTOX( &mp->woffset );
	im->xhseg = mbvtop( nxva );
	im->xhhead = HTOX( &mp->roffset );
# ifdef BUG1
	im->res1[0] = 1;
# endif BUG1
	/*
	** calculate the physical dma address of the initialization message
	** and pass it to controller.
	**
	** Exos version 1.4 requires that a disallowed address be
	** sent first for synchronization (0xffff0000).  Valid addresses
	** must have high byte == 0 (24 bits is maximum phys address)
	*/
	_sendlong( 0x0000FFFFL );
	addr = (long)mbvtop( im ) & 0xffffff; /* get phys addr of init message */
	_sendlong( addr );

	i = 0;
	while ( im->code == 0xff )	/* wait for exos to change 'code' */
		if ( i++ > 100000 )
		{
	dprintf(("nxqsetup: can't init board\n" ));
		    _xnserr = ETIMEDOUT;
		    return (0);
		}
	if ( im->code )
	{
	dprintf(("nxqsetup: bad code\n" ));
		_xnserr = EIO;
		_nxpresent = 0;
		return (0);
	}
	return (1);
}

/* 
** nxmode
**   set the mode of the Execelan interface
*/
_nxmode( newmode )
{
	register MODE		modemsg;
	struct request		req;
	register struct request	*modereq = &req;
	int			timer;	/* for standalone code only - djb */

	modemsg = (MODE)_getmsgbuf();	/* get a message buffer */

	/* Fill in the request struct */

	(MODE)modereq->msg = modemsg;
	modereq->reply = 0;
	modereq->complete = 0;

	/* Fill in the message buffer */

	modemsg->request = NET_MODE;
	modemsg->reqmask = WRITEREQ;
	modemsg->mode = newmode;
	modemsg->id = (long)modereq;	/* link back to request struct	*/
	modemsg->status = NX_BUSY;	/* mark msgbuf as owned by exos	*/
	*PortB = 0;			/* interrupt (signal) controller */

	return (_nxwaitc( modereq ) );
}

/*
** getslot
**   read back the board's ethernet address
*/
_getslot( slotno, a )
struct physnet	*a;
{
	register ADDRS		adrmsg;
	struct request		req;
	register struct request	*adrreq = &req;
	char			reply[ sizeof (struct xmit_msg) ];
	register int		timer;	

	adrmsg = (ADDRS)_getmsgbuf();
	(ADDRS)adrreq->msg = adrmsg;
	adrreq->reply = reply;
	adrreq->complete = 0;
	adrmsg->request = NET_ADDRS;
	adrmsg->reqmask = READREQ;
	adrmsg->slot = slotno;
	adrmsg->id = (long)adrreq;
	adrmsg->status = NX_BUSY;
	*PortB = 0;			

	if ( ! _nxwaitc( adrreq ) )
		return (0);
	adrmsg = (ADDRS)reply;	/* reply[] should now have the result */ 
	bcopy( &adrmsg->addr, a, 6 );
	return (1);
}

/*
** nxstats
**   returns all the network statistics to the user's buffer.
**   The stats are 8 longwords: 32 bytes.
*/
_nxstats( usrbuf )
char *usrbuf;
{
	register STATS		statmsg;
	struct request		req;		/* request struct */
	register struct request	*statreq = &req;
	register int		timer;

	if ( ! _nxpresent )
	{
		_xnserr = ENODEV;
		return (-1);
	}

	statmsg = (STATS)_getmsgbuf();
	(STATS)statreq->msg = statmsg;
	statreq->reply = 0;
	statreq->complete = 0;
	statmsg->request = NET_STATS;
	statmsg->reqmask = READREQ | WRITEREQ;	/* read and reset stats */
	statmsg->nobj = 8;				/* read all stats */
	statmsg->index = 0;				/* start at beginning */
	statmsg->baddr = (long)( mbvtop( _readdatabuf ) );
	statmsg->id = (long)statreq;
	statmsg->status = NX_BUSY;
	*PortB = 0;		

	if ( ! _nxwaitc( statreq ) )
	{
		printf("nxstats: timeout\n");
		return (0);
	}
	bcopy( _readdatabuf, usrbuf, 8 * sizeof (long) );
	return (1);
}

_ResetExcelan() {
	probe(PortA,1);
}

/*
** readenable
**   enable the board to give us some info
*/
_readenable()
{
	READ	readmsg;

	if ( _nxflags & READING )
		return;
	_nxflags |= READING;
	readmsg = (READ)_getmsgbuf();
	(READ)readrequest->msg = readmsg;
	readrequest->reply = readreply;
	readrequest-> complete = 0;

	readmsg->request = NET_READ;
	readmsg->nblocks = 1;
	readmsg->block[ 0 ].len = MAXPACKET;
	readmsg->block[ 0 ].addr = mbvtop( _readdatabuf );
	readmsg->id = (long)readrequest;
	readmsg->status = NX_BUSY;
	*PortB = 0;
}

/*
** nxread
**   read from the excelan board
*/
_nxread()
{
	register READ	replymsg;

	_nxintr();
	if ( readrequest->complete )
	{
		_nxflags &= ~READING;
		replymsg = (READ)( readrequest->reply );
		return (replymsg->block[ 0 ].len );
	}
	return (0);
}

/*
** nxwrite
**   write to the board
*/
_nxwrite( buf, bytes )
register char	*buf;
register int	bytes;
{
	register XMIT		writemsg;
	register int		timer;

	writemsg = (XMIT)_getmsgbuf();

	/* copy user's data into system buf */
	bcopy( buf, writedatabuf, bytes );
	(XMIT)writerequest->msg = writemsg;
	writerequest->reply = 0;		/* don't want the reply msg */
	writerequest->complete = 0;

        writemsg->request = NET_XMIT;
	writemsg->nblocks = 1;
	writemsg->block[0].len = bytes;
	writemsg->block[0].addr = (long)( mbvtop( writedatabuf ) );
	writemsg->id = (long)writerequest;
	writemsg->status = NX_BUSY;
	*PortB = 0;

	return ( _nxwaitc( writerequest ) );
}    

/*
** igetmsgbuf
**    Locate next host-to-exos msgbuf.
**    Return NULL if none available.
**    status byte interpretation:
**	3 - is busy and owned by exos
**	2 - is busy and owned by host
**	1 - is not busy but owned by exos
**	0 - is available
*/
MP
_igetmsgbuf()
{
register	s,
		i;
register MP	mp;

	mp = (MP)XTOH( nxmp->woffset );
	for (i = 0; i < NWBUFS; mp = mp->next )
	if ( mp->msg.status == 0 )
	    goto gotone;
	return ((MP)0);
gotone:
	mp->msg.status = 2;
	mp->msg.res = 0;
	bzero( &mp->msg.pad, MSGLEN );
	return (mp);
}

/*
** getmsgbuf
**   Obtain a host-to-exos msgbuf.
**   Wait if necessary to get one.
*/
MP
_getmsgbuf()
{
	register	s;
	register MP	mp;
	register	timer;

	timer = 50;		/* try 50 times then give up */
	while ( timer-- )
		if ( mp = _igetmsgbuf() )
		    return (mp);
	printf("getmsgbuf failed\n");
	exit( 0 );
}

/*
** nxwaitc
*/
_nxwaitc( rp )
register struct request	*rp;
{
	register long	timer;

	for ( timer = NXMAXBUZZ; --timer >= 0; )
	{
		_nxintr();
		if( rp->complete )
		    return (1);
	}
	dprintf(("nxwaitc: timeout\n" ));

	_xnserr = ETIMEDOUT;
	return (0);
}

/*
** sendlong
**   send out a long word over the net. byte by byte!
*/
_sendlong( n )
long	n;
{
	register int i;

	for ( i = sizeof (long); --i >= 0; )
	{
		while ( *PortB & NX_NOT_READY )
		    ;
		*PortB = (char)n;
		n >>= NBBY;
	}
}
