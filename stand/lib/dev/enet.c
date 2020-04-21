/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/enet.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:28 $
 */

#include	"net.h"
#include	"cntrlr.h"
#include	"dprintf.h"

extern Xhost	_MyHostAddress;
extern char	*_readdatabuf;

/*
** etherwrite
**   write a packet on a specific ethertype.  If the packet is
**   smaller than the minimum packet size, it is extended. The
**   src address is set to MyHostAddress, and the ethertype is
**   set to etype.
*/
_etherwrite( etype, buffer, nbytes )
register int	etype;
register char	buffer[];
register int	nbytes;
{
	register etheader	*etherheader; 
	register int		writecount;

	if ( nbytes < MIN_ENET_PACKET )
		nbytes = MIN_ENET_PACKET;
	etherheader = (etheader *)buffer;
	etherheader->src = _MyHostAddress;
	etherheader->etype = etype; 
	if ( ( writecount = _nxwrite( buffer, nbytes ) ) < 0 )
	{
	dprintf(( "etherwrite: nxwrite failed\n" ));
		printf( "etherwrite: error\n");
	}
	return (nbytes);
}

/*
** etherread
**   read a packet from the ethernet of the given ethertype.
**   This returns READTIMEOUT if no packet of the given 
**   ethertype arrives within the current timeout period. 
**   Packets of other ethertypes are ignored!  The value
**   ERROR is returned on error.
*/
_etherread( etype )
register int	etype;
{
	register int		bytecount;
	register etheader	*etherheader;
	register long		timer;

	timer = 60000;
	_readenable();
	while ( timer-- )
	{
		if ( ( bytecount = _nxread() ) <= 0 )
			continue;
		etherheader = (etheader *)_readdatabuf;
		if ( etherheader->etype == etype )
			return (bytecount);
		else
			_readenable();
	}
	dprintf(( "etherread: timeo\n" ));

	return (READTIMEOUT);
}
