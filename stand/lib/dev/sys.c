/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/sys.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:55 $
 */
#include	"stand.h"
#include	"sysmacros.h"
#include	"dprintf.h"

_devread( io )
register struct iob	*io;
{
	register struct inode *ip;
	register struct buf *bp;
	register dev_t	dev;
	caddr_t mbmap();

	ip = io->i_ip;
	bp = io->i_bp;
	dev = ip->i_dev;
	io->i_error = 0;
	bp->b_error = 0;
	bp->b_flags = B_READ;
	bp->b_dev = dev;
	bp->b_iobase = mbmap( bp->b_iobase, bp->b_bcount);
	(*devsw[major(dev)].dv_strategy)( io, READ );
	if ( bp->b_error != 0 ) {
		io->i_error = bp->b_error;
		dprintf(("ioerror\n"));
		return(-1);
	}
	return(bp->b_bcount);
}

_devwrite( io )
register struct iob	*io;
{

}

_devopen( io, ext, file )
register struct iob	*io;
char			*ext,
			*file;
{
	register struct inode *ip;

	ip = io->i_ip;
	io->i_error = 0;
	return(*devsw[major(ip->i_dev)].dv_open)( io, ext, file );
}

_devclose( io )
register struct iob	*io;
{
	register struct inode *ip;

	ip = io->i_ip;
	(*devsw[major(ip->i_dev)].dv_close)( io );
}

_nullsys( io )
register struct iob	*io;
{
printf("nullsys\n");
	io->i_error = 0;
}

_nullioctl( io, cmd, arg )
register struct iob	*io;
int			cmd;
caddr_t			arg;
{
	io->i_error = 0;
}

iodone( bp )
register struct buf *bp;
{

	bp->b_flags |= B_DONE;
}
