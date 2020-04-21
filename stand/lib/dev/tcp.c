/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/tcp.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:56 $
 */

#include	"stand.h"
#include	"Xns.h"
#include	"cntrlr.h"
#include	"bootp/defs.h"
#include	"cpureg.h"
#include	"common.h"

extern short _nxflags;		/* Excelan status flags */
extern Xhost _MyHostAddress;	/* Ethernet address of this host */

extern char netbuf[];
extern char *netbp;
extern int netbuflen;

int _tcperr;	/* used by lower levels to report errors */

/*
** tcpopen
**
** Try to establish a TFTP pseudo-connection to a server that
** has the file we want.  This may involve invoking the BOOTP
** protocol (RFC 951) to find our IP address and the IP address
** of the server.
*/
_tcpopen( io, ext, file )
struct iob	*io;
char		*ext,
		*file;
{
	register int nbytes;
	char *hostname;

	dprintf(("tcpopen ext %s, file %s\n", ext, file));
	
	tcpinit();

	/*
	** Check for our Internet address in the common area if not
	** already set.
	*/ 
#define IADDR(p)	(*((iaddr_t *)(p)))
	if (btmyiaddr == 0 && IADDR(_commdat->c_iaddr) != 0)
		btmyiaddr = IADDR(_commdat->c_iaddr);

	/*
	** If BOOTP has been invoked before, we must check whether
	** the server name has changed in order to know whether the
	** saved server IP address is still valid for this open.
	** If the name is different, zap the saved address and the
	** corresponding gateway address.
	*/
	if (bootp_sname[0] != '\0' && strcmp(ext, bootp_sname)) {
		bootp_siaddr = 0;
		bootp_giaddr = 0;
	}

	/*
	** Decide whether we need to invoke BOOTP.
	**
	** In order to do TFTP we need IP addresses for our system
	** and for the server.  BOOTP can also be used to translate
	** generic pathnames into fully qualified boot file names.
	** So we invoke BOOTP in the following cases:
	**
	** 1) don't yet know IP address of this system (btmyiaddr)
	** 2) don't yet know IP address for server (bootp_siaddr)
	** 3) requested file name is not fully qualified
	*/
	if (btmyiaddr == 0 || bootp_siaddr == 0 || file[0] != '/') {
		if (bootp( ext, file ) < 0) {
			io->i_error = _tcperr;
			return (-1);
		}
		/*
		** If the file name returned by BOOTP is null, then we asked
		** for a particular server, but he didn't have the file.
		*/
		if (bootp_file[0] == '\0') {
			io->i_error = ENOENT;
			return (-1);
		}
	} else {
		dprintf(( "tcpopen: skipping BOOTP\n" ));
		strcpy( bootp_file, file );
	}
	
	if ( bootp_sname[0] == '\0' ) {
		printf("Network file: tcp:%s from host at %s\n",
			bootp_file, inet_ntoa(ntohl(bootp_siaddr)));
	} else
		printf("Network file: tcp.%s:%s\n", bootp_sname, bootp_file);
	
	/*
	** Now call TFTP to send read request (RRQ)
	*/
	if ( ( nbytes = tftpconnect() ) < 0 ) {
		dprintf(( "tcpopen: connect failed\n" ));
		io->i_error = _tcperr;
		return (-1);
	}

	/*
	** The file may be zero length
	*/
	if ( nbytes == 0 ) {
		io->i_flgs |= F_EOF;
	}

	/*
	** If not EOF, then copy the data to the save buffer for tftpread.
	** We know the data will fit, since the fixed transfer size of
	** TFTP is less than the buffer size.
	*/
	if ( nbytes > 0 ) {
		netbuflen = nbytes;
		netbp = netbuf;
		bcopy( tftp_dataptr, netbuf, nbytes );
	}

	dprintf(("tcpopen: tftp connect returns %d\n", nbytes));

	return (0);
}

/*
** tcpclose
**   close the connection
*/
_tcpclose( io )
struct iob	*io;
{
	dprintf(("tcpclose\n"));

	/*
	 * If the last packet has not been received already, then send
	 * an error packet to stop the server from retrying the send.
	 */
	if ( !tftp_eof )
		tftpabort();
}

/*
** tcpstrategy
**   perform the indicated xfer of data using TFTP.
**   only reading is supported.  tcpstrategy makes no
**   attempt to return the full amount of io->i_count.
**   it only returns what is convenient and the higher
**   level must keep trying.
*/
_tcpstrategy( io, flag )
register struct iob	*io;
int			flag;
{
	register int	nbytes;
	register int	cnt;

	if ( flag == READ ) {
		/*
		** use anything in netbuf first
		*/
		if ( netbuflen ) {
		    cnt = min( netbuflen, io->i_count );
		    bcopy( netbp, io->i_base, cnt );
		    netbp += cnt;
		    netbuflen -= cnt;
		    return( cnt );
		}

		/*
		** netbuf is empty, so read from TFTP
		*/
		if ( ( nbytes = tftpread() ) < 0 ) {
			dprintf(( "tcpstrategy: read failed (errno %d)",
				_tcperr ));
			io->i_error = _tcperr;
			goto errout;
		}

		if ( nbytes == 0 ) {
			io->i_flgs |= F_EOF;
			cnt = 0;
		}

		/*
		** if not EOF, then copy the data to our buffer.
		*/
		if ( nbytes > 0 ) {
		    cnt = min( nbytes, io->i_count );
		    bcopy( tftp_dataptr, io->i_base, cnt );
		    /* copy any left over bytes to our buffer */
		    if ( nbytes > cnt ) {
			netbuflen = nbytes - cnt;
			netbp = netbuf;
		    	bcopy( tftp_dataptr + cnt, netbuf, netbuflen );
		    }
		}
		return (cnt);
	}

errout:
	if (io->i_error == 0)
		io->i_error = EINVAL;
	return (-1);
}


/*
** tcpioctl
*/
_tcpioctl( io, cmd, arg )
register struct iob	*io;
int			cmd;
caddr_t			arg;
{
	dprintf(("tcpioctl\n"));
	io->i_error = 0;
}

/*
** TCP initialization
**
** Called on each device open
*/
tcpinit()
{
	/*
	** If the execlan board has not been initialized, do it
	*/
	if ( ! ( _nxflags & INITED ) )
		if ( !_nxinit() ) {
		    dprintf(( "tcpinit: nxinit failed\n" ));
		    return (-1);
		}
	
	/*
	** Reset ARP state
	*/
	arpwait = 0;
	arpvalid = 0;
	arpqueue = NULL;

	/*
	** Reset netbuf and tcp error status
	*/
	netbuflen = 0;
	_tcperr = 0;

	/*
	** Reset state of TFTP pseudo-connection
	*/
	tftp_blkno = -1;
	tftp_eof = 0;
	tftp_dataptr = NULL;

	/*
	** This is a hack to get around different definitions
	** of Ethernet address structures.
	*/
	bcopy((caddr_t)&_MyHostAddress, (caddr_t)&btmyeaddr, sizeof btmyeaddr);
}
