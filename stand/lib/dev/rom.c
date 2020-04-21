/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/rom.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:47 $
 */
/*
 * Driver to boot from a prom board for manufacturing tests
 */

#include "stand.h"
#include "cpureg.h"

#define DEFAULTADDR	0x200000	/* default address for prom	*/

char *promaddr;

romopen( io, ext, file )
register struct iob *io;
register char *ext;
char *file;
{
	long x;

	if ( *ext == '\0' )
		promaddr = (char *)(SEG_MBMEM + DEFAULTADDR);
	else {
		if ( (x = aton(ext, 16 )) < 0 )
			x = DEFAULTADDR;
		promaddr = (char *)(SEG_MBMEM +  x);
	}
	printf("Open of rom board @ 0x%x (ext is -%s-)\n",promaddr,ext);
	if ( probe( promaddr ) )
		return(0);

	/* no rom board present */
	printf("no ROM board present\n");
	io->i_error = ENXIO;
	return(-1);
}

romclose( io )
register struct iob *io;
{
}

romstrategy( io, flag )
register struct iob *io;
int flag;
{
	register char *from;
	register int count;

	switch ( flag ) {
	    case READ:
		from = io->i_offset + promaddr;
		bcopy( from, io->i_base, io->i_count);
		count = io->i_count;
		break;

	    case WRITE:
	    default:
		io->i_error = ENXIO;   
		count = -1;
		break;
	}
	return( count );
}
