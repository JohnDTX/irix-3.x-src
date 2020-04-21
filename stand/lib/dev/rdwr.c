/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/rdwr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:46 $
 */
#include	"stand.h"
#include	"sysmacros.h"
#include	"dprintf.h"

read( fdesc, buf, count )
int	fdesc,
	count;
char	*buf;
{
	register int i;
	register struct iob	*io;

	if ( ( i = count ) <= 0 )
		return (0);

	/* do stdin first	*/
	if ( fdesc >= 0 & fdesc <= 2 ) {
		do {
			*buf = getchar();
		} while ( --i && *buf++ != '\n' );
		return (count - i);
	}

	/* a file or raw device		*/
	fdesc -= 3;
	if ( fdesc < 0
	     || fdesc >= NFILES
	     || ( ( io = &iobuf[ fdesc ] )->i_flgs & F_ALLOC ) == 0
	     || ( io->i_flgs & F_READ ) == 0 ) {
		errno = EBADF;
		return (-1);
	}

	io->i_base = buf;
	io->i_count = count;

	/*
	 * Handle CPIO files by calling cpio read routine, which
	 * in turn calls rdwr to actually read from the device or file.
	 */
	if ( io->i_flgs & F_CPIO )
		return ( cpio_rw( io, READ ) );

	return ( rdwr( io ) );
}

rdwr( io )
	register struct iob	*io;
{
	register int i;
	register int cnt;
	register int count;
	register struct inode	*ip;
	register struct buf	*bp;

	ip = io->i_ip;
	io->i_error = 0;
	count = i = io->i_count;

	do {
		if ( io->i_flgs & F_EOF )
			return (count-i);

		switch ( ip->i_mode & IFMT ) {

		    case IFCHR:
			if ( io->i_flgs & F_TAPE ) {
				/*
				** Setup for raw device read
				*/
				bp = io->i_bp;
				bp->b_iobase = io->i_base;
				bp->b_bcount = io->i_count;
				dprintf(("rdwr: call devread cnt %d, buf %x\n",
					bp->b_bcount,bp->b_iobase));
				if ( (cnt = _devread(io)) < 0 )
					return(count - i);
			} else {
				cnt = (*devsw[major(ip->i_dev)].dv_strategy)(io, READ);
			}
			break;

		    case IFREG:
		    case IFDIR:
			if ( (cnt = fsread( io )) == 0 )
				return(count - i);
			break;

		    default:
			printf("unknown file type 0x%x\n",ip->i_mode);
			errno = ENXIO;
			return( -1 );
			break;
		}

		if ( io->i_error ) {
			errno = io->i_error;
			return (-1);
		}

		/* now update counters and */
		io->i_base += cnt;
		io->i_count -= cnt;
		io->i_offset += cnt;
	} while ( (i -= cnt) );

	return (count);
}


/* XXX need to write		*/
write( fdesc, buf, count )
int	fdesc,
	count;
char	*buf;
{
	register 		i;
	register struct iob	*io;

	errno = 0;
	if ( fdesc >= 0 && fdesc <= 2 )
	{
		i = count;
		while (i--)
			putchar(*buf++);
		return (count);
	}
	fdesc -= 3;
	if ( fdesc < 0 || fdesc >= NFILES ||
	   ( ( io = &iobuf[ fdesc ] )->i_flgs & F_ALLOC ) == 0 )
	{
		errno = EBADF;
		return (-1);
	}
	if ( ( io->i_flgs & F_WRITE ) == 0 )
	{
		errno = EBADF;
		return (-1);
	}
}
