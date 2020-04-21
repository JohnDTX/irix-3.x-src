/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/close.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:19 $
 */

#include	"stand.h"

close( fdesc )
register	fdesc;
{
	register struct iob	*io;

	fdesc -= 3;
	if ( fdesc < 0
	    || fdesc >= NFILES
	    || ( ( io = &iobuf[ fdesc ] )->i_flgs & F_ALLOC ) == 0 )
	{
		errno = EBADF;
		return (-1);
	}
	_devclose( io );
	io->i_flgs = 0;
	return (0);
}
