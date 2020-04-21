/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/xns.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:59 $
 */

#include	"stand.h"
#include	"Xns.h"
#include	"cntrlr.h"
#include	"dprintf.h"

#define	NTRIES		8	/* number of tries to try to find a host    */
#define MAXIDPSIZE	1500

int	_xnserr;		/* holds errno from lower levels	*/

/*
** Netbuf
**
** This buffer and associated counters are used to manage data
** that arrives in a packet that exceeds the current read request.
** It is shared by the xns and tcp strategy routines.
*/
int	netbuflen;		/* count of characters in netbuf	*/
char	*netbp;
char	netbuf[MAXIDPSIZE];


/*
** xnsopen
**   open the sequenced packet protocol
**   We verify the host is responding, and the file is present,
**   then establish the sequenced protocol socket.
*/
_xnsopen( io, ext, file )
struct iob	*io;
char		*ext,
		*file;
{
	Xaddr		bootaddr;
	char		*info;
	char		strbuf[ 50 ];
	register int	conflen;

	_xnserr = 0;
	netbuflen = 0;

	/*
	** form a string the boot server can recognize
	*/
	_netstr( strbuf, 50, ext, file );


	/*
	** see if the host is responding???
	*/
	if ( _findhost( strbuf, &bootaddr.host, &info ) < 0 )
	{
	dprintf(( "xnsopen: findhost failed\n" ));
		io->i_error = _xnserr;
		return (-1);
	}

	/*
	** host is responding, but the requested file doesn't exist
	*/
	if( *info == SERV_NOFILE )
	{
		io->i_error = ENOENT;
		return (-1);
	}

	printf("Network file: %s\n",info+2);

	/*
	** open the sequenced packet connection
	*/
	bootaddr.socket = BOOTSOCKET;
	if( ! _seqopen( BOOTSOCKET, bootaddr ) )
	{
	dprintf(( "xnsopen: SEQ open failed\n" ));
		io->i_error = _xnserr;
		return (-1);
	}

	/* hell if I know?????	*/
	conflen = strlen(strbuf) + 3;	/* why + 3?????	*/


	/*
	** write the open request to the sequenced socket
	*/
	if( _seqwrite( strbuf, conflen ) != conflen )
	{
	dprintf(( "xnsopen: SEQ write failed\n" ));
		io->i_error = EIO;
		_seqclose();
		return (-1);
	}


	return (0);
}

/*
** xnsclose
**   close the connection
*/
_xnsclose( io )
struct iob	*io;
{
	_xnserr = 0;

	_seqclose();
}

/*
** xnsstrategy
**   perform the indicated xfer of data.
**   We only support reading!
*/
_xnsstrategy( io, flag )
register struct iob	*io;
int			flag;
{
	extern char	*_seqdataptr;
	register int	nbytes;
	register int	cnt;

	_xnserr = 0;
	if ( flag == READ )
	{
		/* copy out of netbuf first */
		if ( netbuflen ) {
		    cnt = min( netbuflen, io->i_count );
		    bcopy( netbp, io->i_base, cnt );
		    netbp += cnt;
		    netbuflen -= cnt;
		    return( cnt );
		}
		/*
		** read for the 'seq' socket.
		** if we get an error, give-up, but if the read times out
		** just keep trying.
		*/
		while ( ( nbytes = _seqread() ) < 0 ) {
			if ( nbytes == -1 ) {
				dprintf(( "xnsstrat: read failed\n" ));
				io->i_error = _xnserr;
				break;
			}
		}
		if ( nbytes == 0 ) {
			io->i_flgs |= F_EOF;
			cnt = 0;
		}

		/*
		 * if not EOF, then copy the data to our buffer.
		 */
		if ( nbytes > 0 ) {
		    cnt = min( nbytes, io->i_count );
		    bcopy( _seqdataptr, io->i_base, cnt );
		    /* copy any left over bytes to our buffer */
		    if ( nbytes > cnt ) {
			netbuflen = nbytes - cnt;
			netbp = netbuf;
		    	bcopy( _seqdataptr + cnt, netbuf, netbuflen );
		    }
		}
		return (cnt);
	}

	io->i_error = EINVAL;
	return (-1);
}

/*
** findhost
**   try to find a host who will respond.
*/
_findhost( config, _host, _info)
char	*config;
Xhost	*_host;
char	**_info;
{
	register int	ntries;

	/*
	** for some number of tries, broadcast over the net
	** and see if any host replies
	*/
	for ( ntries = NTRIES; --ntries >= 0; )
	{
		if ( _bcast( config ) < 0 )
		    return (-1);

		if ( _getreply( _host, _info ) >= 0 )
		    return (0);
	}

	dprintf(( "findhost: timeo\n" ));
	_xnserr = ETIMEDOUT;
	return (-1);
}

Xhost _BcastAddress = { 0xFFFF, 0xFFFF, 0xFFFF };

/*
** bcast
**   broadcat a message over the net
**   Returns 0 if we get an answer, -1 if we don't.
*/
_bcast( str )
char	*str;
{
    Xserverreq		serverreq;
    register Xserverreq *Req = &serverreq;

	if ( ! _seqinit() )
	{
	dprintf(( "bcast: cannot in SEQ\n" ));
		return (-1);
	}

	Req->etherheader.dst = _BcastAddress;
	Req->exchid = 1;
	strncpy( Req->infostr, str, INFOSTRLEN );
	_etherwrite( SG_BOUNCE, Req, sizeof( Xserverreq ) );

	return (0);
}

/*
** getreply
**   get the answering reply from a host.
*/
_getreply( _host, _info )
Xhost	*_host;
char	**_info;
{
    extern Xhost	_MyHostAddress;
    extern char		*_readdatabuf;
    register int	nbytes;

	for( ;; )
	{
		register Xserverreq *Reply = (Xserverreq *)_readdatabuf;

		if ( ( nbytes = _etherread( SG_BOUNCE ) ) == READTIMEOUT )
		{
			dprintf(( "getreply: read timeo\n" ));
		    return (-1);
		}

		if ( _HostSame( &_MyHostAddress, &Reply->etherheader.dst )
		     && Reply->exchid == 1
		     && ( Reply->infostr[ 0 ] == SERV_NOFILE
		     || Reply->infostr[ 0 ] == SERV_REPLY ) )
		{
		    *_host = Reply->etherheader.src;
		    *_info = Reply->infostr;
		    return (0);
		}
	}
}

_netstr( str, len, ext, file )
char	*str;
int	len;
char	*ext,
	*file;
{
	register char	*tp,
			*last;
	extern char		*strlcpy();

	dprintf(("netstr: ext %s, file %s",ext,file));

	tp = str;
	last = tp + len - 7;	/* make sure our additions fit! */

	*tp++ = SERV_SENDFILE;
	last++;
	*tp++ = ':';

	tp = strlcpy( tp, last, ext );
	last++;
	*tp++ = ':';

	tp = strlcpy( tp, last, file );

	/*
	** add some extra nulls, why????
	*/
	*tp++ = 0;
	*tp++ = 0;
	*tp++ = 0;
	*tp   = 0;
}

/*
 * strlcpy() --
 * copy up to N bytes from src to tgt;
 * return a pointer to the last tgt byte.
 */
static
char *
strlcpy( tgt, last, src )
register char	*tgt,
		*last;
register char	*src;
{
	register char	*last;

	for ( ; tgt != last; tgt++ )
		if ( ( *tgt = *src++ ) == 0 )
		    break;

	return (tgt);
}
